/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/connection.h>
#include <cloysterhpc/network.h>
#include <cloysterhpc/services/log.h>

#include <arpa/inet.h> /* inet_*() functions */
#include <boost/asio.hpp>
#include <ifaddrs.h>
#include <regex>
#include <resolv.h>
#include <string>
#include <vector>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

Network::Network()
    : Network(Profile::External)
{
}

Network::Network(Profile profile)
    : Network(profile, Type::Ethernet)
{
}

Network::Network(Profile profile, Type type)
    : m_profile(profile)
    , m_type(type)
{
}

Network::Network(Profile profile, Type type, const std::string& ip,
    const std::string& subnetMask, const std::string& gateway,
    const uint16_t& vlan, const std::string& domainName,
    const std::vector<address>& nameserver)
    : Network(profile, type)
{
    setAddress(ip);
    setSubnetMask(subnetMask);
    setGateway(gateway);
    setVLAN(vlan);
    setDomainName(domainName);
    setNameservers(nameserver);
}

Network::Network(Profile profile, Type type, const std::string& ip,
    const std::string& subnetMask, const std::string& gateway,
    const uint16_t& vlan, const std::string& domainName,
    const std::vector<std::string>& nameserver)
    : Network(profile, type)
{
    setAddress(ip);
    setSubnetMask(subnetMask);
    setGateway(gateway);
    setVLAN(vlan);
    setDomainName(domainName);
    setNameservers(nameserver);
}

//// TODO: Check for std::move support on const member data
////  * https://lesleylai.info/en/const-and-reference-member-variables/
// Network::Network(Network&& other) noexcept
//         : m_profile{other.m_profile}
//         , m_type{other.m_type}
//         , m_address{other.m_address}
//         , m_subnetMask{other.m_subnetMask}
//         , m_gateway{other.m_gateway}
//         , m_vlan{other.m_vlan}
//         , m_domainName{std::move(other.m_domainName)}
//         , m_nameservers{std::move(other.m_nameservers)}
//{}

const Network::Profile& Network::getProfile() const { return m_profile; }

const Network::Type& Network::getType() const { return m_type; }

/* TODO: Implement checks
 *  - Subnet correct size
 *  - Overload for different inputs (string and int)
 *  - Check if network address and gateway are inside the mask
 */
address Network::getAddress() const { return m_address; }

void Network::setAddress(const address& ip)
{
    const address unspecifiedAddress = boost::asio::ip::make_address("0.0.0.0");

    if (ip == unspecifiedAddress)
        throw std::runtime_error("IP address cannot be 0.0.0.0");

    m_address = ip;
    LOG_TRACE("{} network address set to {}", magic_enum::enum_name(m_profile),
        m_address.to_string());
}

void Network::setAddress(const std::string& ip)
{
    try {
        setAddress(boost::asio::ip::make_address(ip));
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid IP address");
    }
}

address Network::fetchAddress(const std::string& interface)
{
    struct in_addr addr { };
    struct in_addr netmask { };
    struct in_addr network { };

    if (inet_aton(
            Connection::fetchAddress(interface).to_string().c_str(), &addr)
        == 0)
        throw std::runtime_error("Invalid IP address");
    if (inet_aton(fetchSubnetMask(interface).to_string().c_str(), &netmask)
        == 0)
        throw std::runtime_error("Invalid subnet mask address");

    network.s_addr = addr.s_addr & netmask.s_addr;

    auto result = boost::asio::ip::make_address(inet_ntoa(network));

    LOG_TRACE(
        "Got address {} from interface {}", result.to_string(), interface);

    return result;
}

address Network::getSubnetMask() const { return m_subnetMask; }

void Network::setSubnetMask(const address& subnetMask)
{
    //@TODO Use class network_v4 instead
    if (!cidr.contains(subnetMask.to_string()))
        throw std::runtime_error("Invalid subnet mask");

    m_subnetMask = subnetMask;
}

void Network::setSubnetMask(const std::string& subnetMask)
{
    try {
        setSubnetMask(boost::asio::ip::make_address(subnetMask));
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid subnet mask");
    }
}

address Network::fetchSubnetMask(const std::string& interface)
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(
            fmt::format("Cannot get interfaces: {}", std::strerror(errno)));

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_netmask == nullptr)
            continue;

        if (ifa->ifa_netmask->sa_family != AF_INET)
            continue;

        // TODO: Check for leaks since we can't run freeifaddrs before return
        if (std::strcmp(ifa->ifa_name, interface.c_str()) == 0) {
            auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);

            if (inet_ntoa(sa->sin_addr) == nullptr)
                continue;

            address result
                = boost::asio::ip::make_address(inet_ntoa(sa->sin_addr));

#ifndef NDEBUG
            LOG_TRACE("Got subnet mask address {} from interface {}",
                result.to_string(), interface);
#endif
            freeifaddrs(ifaddr);
            return result;
        }
    }

    freeifaddrs(ifaddr);
    return {};
    throw std::runtime_error(fmt::format(
        "Interface {} does not have a netmask address defined", interface));
}

address Network::calculateAddress(const address& connectionAddress)
{

    if (m_subnetMask.is_unspecified()) {
        throw std::runtime_error(
            fmt::format("Network must have a specified subnet mask to "
                        "calculate the address"));
    }

    struct in_addr ip_addr { };
    struct in_addr subnet_addr { };

    inet_aton(connectionAddress.to_string().c_str(), &ip_addr);
    inet_aton(m_subnetMask.to_string().c_str(), &subnet_addr);
    return boost::asio::ip::make_address(
        inet_ntoa({ ip_addr.s_addr & subnet_addr.s_addr }));
}

address Network::calculateAddress(const std::string& connectionAddress)
{
    try {
        return calculateAddress(
            boost::asio::ip::make_address(connectionAddress));
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid ip address");
    }
}

address Network::getGateway() const { return m_gateway; }

void Network::setGateway(const address& gateway) { m_gateway = gateway; }

void Network::setGateway(const std::string& gateway)
{
    try {
        setGateway(boost::asio::ip::make_address(gateway));
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid gateway");
    }
}

// FIXME: It's fetching the broadcast address instead
address Network::fetchGateway(const std::string& interface)
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(
            fmt::format("Cannot get interfaces: {}", std::strerror(errno)));

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_dstaddr == nullptr)
            continue;

        if (ifa->ifa_dstaddr->sa_family != AF_INET)
            continue;

        // TODO: Check for leaks since we can't run freeifaddrs before return
        if (std::strcmp(ifa->ifa_name, interface.c_str()) == 0) {
            address result
                = boost::asio::ip::make_address(ifa->ifa_dstaddr->sa_data);

            if (result.is_unspecified())
                continue;

#ifndef NDEBUG
            LOG_TRACE("Got gateway address {} from interface {}",
                result.to_string(), interface);
#endif

            return result;
        }
    }

    freeifaddrs(ifaddr);
    return {};
    throw std::runtime_error(fmt::format(
        "Interface {} does not have a gateway IP address defined", interface));
}

uint16_t Network::getVLAN() const { return m_vlan; }

void Network::setVLAN(uint16_t vlan)
{
    if (vlan >= 4096)
        throw std::out_of_range("VLAN value must be less than 4096.");
    m_vlan = vlan;
}

const std::string& Network::getDomainName() const { return m_domainName; }

void Network::setDomainName(const std::string& domainName)
{
    if (domainName.size() > 255)
        throw std::length_error("Domain name exceeds the maximum allowed "
                                "length of 255 characters.");

#if __cpp_lib_starts_ends_with >= 201711L
    if (domainName.starts_with('-') or domainName.ends_with('-'))
#else
    if (boost::algorithm::starts_with(domainName, "-")
        or boost::algorithm::ends_with(domainName, "-"))
#endif
        throw std::runtime_error("Invalid hostname");

    /* Check if string has only digits */
    if (std::regex_match(domainName, std::regex("^[0-9]+$")))
        throw std::invalid_argument(
            "Domain name should not consist solely of numeric digits.");

    /* Check if it's not only alphanumerics and - */
    if (!(std::regex_match(domainName, std::regex("^[A-Za-z0-9-.]+$"))))
        throw std::invalid_argument(
            "Domain name contains invalid characters. Only alphanumeric "
            "characters and hyphens are allowed.");

    m_domainName = domainName;
}

std::string Network::fetchDomainName()
{
    char domainName[256]; // Adjust the buffer size as needed
    if (getdomainname(domainName, sizeof(domainName)) == -1)
        throw std::runtime_error("Failed to fetch domain name");

    LOG_TRACE("Got domain name {}", domainName)

    return std::string(domainName);
}

/* TODO: Check return type
 *  - We can't return const (don't know exactly why)
 */
std::vector<address> Network::getNameservers() const
{
    std::vector<address> returnVector;
    for (const auto& nameserver : std::as_const(m_nameservers))
        returnVector.emplace_back(nameserver);

    return returnVector;
}

void Network::setNameservers(const std::vector<address>& nameservers)
{
    if (nameservers.empty())
        return;

    m_nameservers.reserve(nameservers.size());

#if __cplusplus < 202002L
    size_t i = 0;
    for (const auto& ns : std::as_const(nameservers)) {
#else
    for (std::size_t i = 0; const auto& ns : std::as_const(nameservers)) {
#endif
        m_nameservers.push_back(ns);
    }
}

void Network::setNameservers(const std::vector<std::string>& nameservers)
{
    std::vector<address> formattedNameservers;
    for (std::size_t i = 0; i < nameservers.size(); i++) {
        formattedNameservers.emplace_back(
            boost::asio::ip::make_address(nameservers[i]));
    }

    setNameservers(formattedNameservers);
}

std::vector<address> Network::fetchNameservers()
{
    std::vector<address> nameservers;
    if (res_init() == -1)
        throw std::runtime_error("Failed to initialize domain name resolution");

    nameservers.reserve(static_cast<std::size_t>(_res.nscount));
    for (const auto& ns : _res.nsaddr_list) {
        address formattedNs
            = boost::asio::ip::make_address_v4(inet_ntoa(ns.sin_addr));
        if (formattedNs.to_string() == "0.0.0.0")
            continue;
        nameservers.emplace_back(formattedNs);
    }

    LOG_TRACE("Got nameservers {}", nameservers.data()->to_string())
    return nameservers;
}

#ifndef NDEBUG
void Network::dumpNetwork() const
{
    LOG_DEBUG("Profile: {}", magic_enum::enum_name(m_profile))
    LOG_DEBUG("Type: {}", magic_enum::enum_name(m_type))
}
#endif
