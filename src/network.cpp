/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network.h"
#include "services/log.h"
#include "connection.h"

#include <vector>
#include <string>
#include <regex>
#include <arpa/inet.h> /* inet_*() functions */
#include <ifaddrs.h>
#include <resolv.h>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

Network::Network()
    : Network(Profile::External) {}

Network::Network(Profile profile)
    : Network(profile, Type::Ethernet) {}

Network::Network(Profile profile, Type type)
    : m_profile(profile),
      m_type(type) {}

Network::Network(Profile profile, Type type, const std::string& address,
                 const std::string& subnetMask, const std::string& gateway,
                 const uint16_t& vlan, const std::string& domainName,
                 const std::vector<std::string>& nameserver)
    : Network(profile, type) {

    setAddress(address);
    setSubnetMask(subnetMask);
    setGateway(gateway);
    setVLAN(vlan);
    setDomainName(domainName);
    setNameservers(nameserver);
}

//// TODO: Check for std::move support on const member data
////  * https://lesleylai.info/en/const-and-reference-member-variables/
//Network::Network(Network&& other) noexcept
//        : m_profile{other.m_profile}
//        , m_type{other.m_type}
//        , m_address{other.m_address}
//        , m_subnetMask{other.m_subnetMask}
//        , m_gateway{other.m_gateway}
//        , m_vlan{other.m_vlan}
//        , m_domainName{std::move(other.m_domainName)}
//        , m_nameservers{std::move(other.m_nameservers)}
//{}

const Network::Profile& Network::getProfile () const {
    return m_profile;
}

const Network::Type& Network::getType () const {
    return m_type;
}

/* TODO: Implement checks
 *  - Subnet correct size
 *  - Overload for different inputs (string and int)
 *  - Check if network address and gateway are inside the mask
 */
std::string Network::getAddress() const {
    if (inet_ntoa(m_address) == nullptr)
        throw;
    return inet_ntoa(m_address);
}

void Network::setAddress(const std::string& address) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        throw; //return -1; /* Invalid IP Address */
}

std::string Network::fetchAddress(const std::string& interface) {
    struct in_addr addr{}, netmask{}, network{};

    // TODO: Fix exceptions
    if (inet_aton(Connection::fetchAddress(interface).c_str(), &addr) == 0)
        return {};
        //throw std::runtime_error("Invalid IP address");
    if (inet_aton(fetchSubnetMask(interface).c_str(), &netmask) == 0)
        return {};
        //throw std::runtime_error("Invalid subnet mask address");

    network.s_addr = addr.s_addr & netmask.s_addr;

    return (inet_ntoa(network));
}


std::string Network::getSubnetMask() const {
    if (inet_ntoa(m_subnetMask) == nullptr)
        throw;
    return inet_ntoa(m_subnetMask);
}

void Network::setSubnetMask(const std::string& subnetMask) {
    if (inet_aton(subnetMask.c_str(), &this->m_subnetMask) == 0)
        throw; //return -1; /* Invalid IP Address */
}

std::string Network::fetchSubnetMask(const std::string& interface) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(fmt::format(
                "Cannot get interfaces: {}", std::strerror(errno)));

    for (ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next) {
        if (ifa->ifa_netmask == nullptr)
            continue;

        if (ifa->ifa_netmask->sa_family != AF_INET)
            continue;

        // TODO: Check for leaks since we can't run freeifaddrs before return
        if (std::strcmp(ifa->ifa_name, interface.c_str()) == 0) {
            auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);
            //freeifaddrs(ifaddr);

            if (inet_ntoa(sa->sin_addr) == nullptr)
                continue;

#ifndef NDEBUG
            LOG_TRACE("Got subnet mask address {} from interface {}",
                      inet_ntoa(sa->sin_addr), interface);
#endif

            return inet_ntoa(sa->sin_addr);
        }
    }

    freeifaddrs(ifaddr);
    return {};
    throw std::runtime_error(fmt::format(
            "Interface {} does not have a netmask address defined", interface));
}

std::string Network::getGateway() const {
    if (inet_ntoa(m_gateway) == nullptr)
        throw;
    return inet_ntoa(m_gateway);
}

void Network::setGateway(const std::string& gateway) {
    if (inet_aton(gateway.c_str(), &this->m_gateway) == 0)
        throw; //return -1; /* Invalid IP Address */
}

// FIXME: It's fetching the broadcast address instead
std::string Network::fetchGateway(const std::string &interface) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(fmt::format(
                "Cannot get interfaces: {}", std::strerror(errno)));

    for (ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next) {
        if (ifa->ifa_dstaddr == nullptr)
            continue;

        if (ifa->ifa_dstaddr->sa_family != AF_INET)
            continue;

        // TODO: Check for leaks since we can't run freeifaddrs before return
        if (std::strcmp(ifa->ifa_name, interface.c_str()) == 0) {
            auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_dstaddr);
            //freeifaddrs(ifaddr);

            if (inet_ntoa(sa->sin_addr) == nullptr)
                continue;

#ifndef NDEBUG
            LOG_TRACE("Got gateway address {} from interface {}",
                      inet_ntoa(sa->sin_addr), interface);
#endif

            return inet_ntoa(sa->sin_addr);
        }
    }

    freeifaddrs(ifaddr);
    return {};
    throw std::runtime_error(fmt::format(
            "Interface {} does not have a gateway IP address defined", interface));
}

uint16_t Network::getVLAN() const {
    return m_vlan;
}

void Network::setVLAN(uint16_t vlan) {
    if (vlan >= 4096)
        throw;
    m_vlan = vlan;
}

const std::string& Network::getDomainName() const {
    return m_domainName;
}

void Network::setDomainName(const std::string& domainName) {
    if (domainName.size() > 255)
        throw;

#if __cpp_lib_starts_ends_with >= 201711L
    if (domainName.starts_with('-') or domainName.ends_with('-'))
#else
    if (boost::algorithm::starts_with(domainName, "-") or
        boost::algorithm::ends_with(domainName, "-"))
#endif
        throw std::runtime_error("Invalid hostname");

    /* Check if string has only digits */
    if (std::regex_match(domainName, std::regex("^[0-9]+$")))
        throw;

    /* Check if it's not only alphanumerics and - */
    if (!(std::regex_match(domainName, std::regex("^[A-Za-z0-9-.]+$"))))
        throw;

    m_domainName = domainName;
}

std::string Network::fetchDomainName() {
    if (res_init() == -1)
        throw std::runtime_error("Failed to initialize domain name resolution");

    return _res.defdname; // TODO: Seems to be a deprecated call
}

/* TODO: Check return type
 *  - We can't return const (don't know exactly why)
 */
std::vector<std::string> Network::getNameservers() const {
    std::vector<std::string> returnVector;
    for (const auto& nameserver : std::as_const(m_nameservers))
        returnVector.emplace_back(inet_ntoa(nameserver));

    return returnVector;
}

/* TODO: Test this */
void Network::setNameservers(const std::vector<std::string>& nameservers) {
    if (nameservers.empty())
        return;

    m_nameservers.reserve(nameservers.size());
    struct in_addr aux {};

#if __cplusplus < 202002L
    size_t i = 0;
    for (const auto& ns : std::as_const(nameservers)) {
#else
    for (std::size_t i = 0 ; const auto& ns : std::as_const(nameservers)) {
#endif
        m_nameservers.push_back(aux); // aux may have garbage on it
        if (inet_aton(ns.c_str(), &this->m_nameservers[i++]) == 0)
            throw std::runtime_error(fmt::format(
                    "Cannot add {} as a nameserver", ns));
    }
}

std::vector<std::string> Network::fetchNameservers() {
    std::vector<std::string> nameservers;
    if (res_init() == -1)
        throw std::runtime_error("Failed to initialize domain name resolution");

    nameservers.reserve(static_cast<std::size_t>(_res.nscount));
    for (const auto& ns : _res.nsaddr_list) {
        if (std::string{inet_ntoa(ns.sin_addr)} == "0.0.0.0")
            continue;
        nameservers.emplace_back(inet_ntoa(ns.sin_addr));
    }

    return nameservers;
}

#ifndef NDEBUG
void Network::dumpNetwork() const {
    LOG_DEBUG("Profile: {}", magic_enum::enum_name(m_profile));
    LOG_DEBUG("Type: {}", magic_enum::enum_name(m_type));
}
#endif
