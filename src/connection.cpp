/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/system/detail/errc.hpp>
#include <cloysterhpc/connection.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/network.h>
#include <cloysterhpc/services/log.h>

#include <expected>
#include <regex>
#include <set>
#include <string>

#include <boost/algorithm/string.hpp>
#include <utility>

#include <arpa/inet.h> /* inet_*() functions */
#include <cstring>
#include <ifaddrs.h> /* getifaddrs() */

#include <fmt/format.h>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

Connection::Connection(Network* network)
    : m_network(network)
{

    if (network->getType() == Network::Type::Infiniband)
        setMTU(2044);
}

Connection::Connection(Network* network,
    std::optional<std::string_view> interface,
    std::optional<std::string_view> mac, const std::string& ip)
    : m_network(network)
{

    if (interface.has_value())
        setInterface(interface.value());

    if (mac.has_value())
        setMAC(mac.value());

    setAddress(ip);

    if (network->getType() == Network::Type::Infiniband)
        setMTU(2044);
}

// Connection::Connection(const Connection& other)
//     : m_network{other.m_network}
//     , m_interface{other.m_interface}
//     , m_mac{other.m_mac}
//     , m_address{other.m_address}
//{}
//
// Connection::Connection(Connection&& other) noexcept
//     : m_network{other.m_network}
//     , m_interface{std::move(other.m_interface)}
//     , m_mac{std::move(other.m_mac)}
//     , m_address{other.m_address}
//{}

std::optional<std::string_view> Connection::getInterface() const
{
    return m_interface;
}

void Connection::setInterface(std::string_view interface)
{
    LOG_DEBUG("Checking if interface {} exists", interface)

    if (interface == "lo")
        throw std::runtime_error("Cannot use the loopback interface");

    /* TODO: Use smart pointers */
    /* Code based on getifaddrs(3) man page */
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(
            fmt::format("Cannot get interface: {}\n", std::strerror(errno)));

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        // TODO: Since we are already here, get the MAC Address from sa_data and
        //       add it to m_mac.
        if (interface == ifa->ifa_name) {
            m_interface = interface;

            freeifaddrs(ifaddr);
            return;
        }
    }

    freeifaddrs(ifaddr);

    throw std::runtime_error(
        fmt::format("Cannot find network interface {}", interface));
}

std::vector<std::string> Connection::fetchInterfaces()
{
    struct ifaddrs *ifaddr, *ifa;
    std::vector<std::string> interfaces;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(
            fmt::format("Cannot get interfaces: {}\n", std::strerror(errno)));

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (std::strcmp(ifa->ifa_name, "lo") == 0)
            continue;

        interfaces.emplace_back(ifa->ifa_name);
    }

    // TODO: It must have a better way to remove duplicates instead of creating
    //       a set to filter out them. Perhaps changing the return type?
    std::set<std::string> aux(interfaces.begin(), interfaces.end());
    interfaces.clear();
    interfaces.reserve(aux.size());
    interfaces.assign(aux.begin(), aux.end());

    return interfaces;
}

std::optional<std::string_view> Connection::getMAC() const { return m_mac; }

void Connection::setMAC(std::string_view mac)
{
    auto validation = Connection::validateMAC(mac);
    if (validation.has_value())
        m_mac = boost::algorithm::to_lower_copy(std::string { mac });
    else
        throw std::runtime_error { validation.error() };
}

std::expected<bool, std::string> Connection::validateMAC(
    std::string_view address)
{
    LOG_DEBUG("Checking MAC address: {}", address)
    if ((address.size() != 12) && (address.size() != 14)
        && (address.size() != 17))
        return std::unexpected("Invalid MAC address size");

    // This pattern validates whether an ADDRESS address is valid or not.
    static std::regex pattern(
        "^("
        "(?:[0-9A-Fa-f]{2}[:-]){5}[0-9A-Fa-f]{2}" // Matches ADDRESS address
                                                  // with colons or hyphens
        "|"
        "(?:[0-9a-fA-F]{4}\\.[0-9a-fA-F]{4}\\.[0-9a-fA-F]{4}" // Matches Cisco
                                                              // ADDRESS format
        "))$");

    // regex_match cannot work with std::string_view
    if (std::string tempString { address }; regex_match(tempString, pattern))
        return true;
    else
        return std::unexpected("Invalid MAC address format");
}

const address Connection::getAddress() const { return m_address; }

void Connection::setAddress(const address& ip)
{
    const address unspecifiedAddress = boost::asio::ip::make_address("0.0.0.0");

    if (ip == unspecifiedAddress)
        throw std::runtime_error("IP address cannot be 0.0.0.0");

    m_address = ip;
}

void Connection::setAddress(const std::string& ip)
{
    try {
        setAddress(boost::asio::ip::make_address(ip));
    } catch (boost::system::system_error& e) {
        throw std::runtime_error("Invalid IP address");
    }
}

void Connection::incrementAddress(const std::size_t increment) noexcept
{
    // TODO increment address with boost
    // m_address.s_addr += increment;
}

address Connection::fetchAddress(const std::string& interface)
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error(
            fmt::format("Cannot get interfaces: {}", std::strerror(errno)));

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        // TODO: Check for leaks since we can't run freeifaddrs before return
        if (std::strcmp(ifa->ifa_name, interface.c_str()) == 0) {
            auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);

            if (inet_ntoa(sa->sin_addr) == nullptr)
                continue;

            boost::system::error_code ec;
            address result
                = boost::asio::ip::make_address(inet_ntoa(sa->sin_addr), ec);

            if (ec.value() != boost::system::errc::success) {
                LOG_TRACE("interface {} returned an error while getting "
                          "address ({}): {}",
                    interface, inet_ntoa(sa->sin_addr), ec.message());
                continue;
            }

            LOG_TRACE("Got address {} from interface {}", result.to_string(),
                interface);

            return result;
        }
    }

    freeifaddrs(ifaddr);
    return {};
    throw std::runtime_error(fmt::format(
        "Interface {} does not have an IP address defined", interface));
}

const std::string& Connection::getHostname() const { return m_hostname; }

void Connection::setHostname(const std::string& hostname)
{
    if (hostname.size() > 63)
        throw std::range_error("Hostname cannot be bigger than 64 characters");

    if (hostname.starts_with('-') or hostname.ends_with('-'))
        throw std::runtime_error("Hostname cannot start or end with dashes");

    /* Check if string has only digits */
    if (std::regex_match(hostname, std::regex("^[0-9]+$")))
        throw std::runtime_error("Hostname has only digits");
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(hostname, std::regex("^[A-Za-z0-9-]+$"))))
        throw std::runtime_error("Invalid character on hostname");

    m_hostname = hostname;
}

const std::string& Connection::getFQDN() const { return m_fqdn; }

void Connection::setFQDN(const std::string& fqdn)
{
    if (fqdn.size() > 255)
        throw std::runtime_error(
            "Full qualified domain name cannot exceed 255 characters");

    m_fqdn = fqdn;
}

// TODO: Check if this return is a best practice; Network is a unique_ptr;
//  * should we use gsl::not_null in the return type.
const Network* Connection::getNetwork() const { return m_network; }

std::uint16_t Connection::getMTU() const { return m_mtu; }

void Connection::setMTU(std::uint16_t mtu)
{
    // RFC791 states that 576 would be the practical minimum for Internet (IPv4)
    // networks, but we consider the rules for IPv6 here to keep compatibility.
    // RFC2460 says that IPv6 requires a minimum of 1280 bytes.
    //
    // https://www.rfc-editor.org/rfc/rfc791
    // https://www.rfc-editor.org/rfc/rfc2460
    //
    // RFC4391 states that 2044 is the minimum required for Infiniband networks.
    // https://www.rfc-editor.org/rfc/rfc4391.html

    switch (m_network->getType()) {
        case Network::Type::Ethernet:
            if (mtu < 1280) {
                throw std::runtime_error(
                    "Ethernet MTU size must be higher or equal to 1280");
            }
            break;
        case Network::Type::Infiniband:
            if (mtu < 2044) {
                throw std::runtime_error(
                    "Infiniband MTU size must be higher or equal to 2044");
            }
            break;
    }

    m_mtu = mtu;
}

#ifndef NDEBUG
void Connection::dumpConnection() const
{
    LOG_DEBUG("Dumping Connection Info:")
    LOG_DEBUG("Connection with Network: {} ({})",
        cloyster::utils::enums::toString(m_network->getProfile()),
        cloyster::utils::enums::toString(m_network->getType()));

    LOG_DEBUG("Interface: {}", m_interface.value_or("NONE"))
    LOG_DEBUG("MAC Address: {}", m_mac.value_or("NONE"))
    LOG_DEBUG("IP Address: {}", getAddress().to_string())

    LOG_DEBUG("===================================")
}
#endif

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

TEST_SUITE("Test MAC address validity")
{
    Network network;
    Connection connection = Connection(&network);

    TEST_CASE("Length Issues")
    {
        CHECK_THROWS(connection.setMAC("ab:cd:ef:01:23")); // Too short
        CHECK_THROWS(connection.setMAC("ab:cd:ef:01:23:45:67")); // Too long
    }

    TEST_CASE("Invalid Separators")
    {
        // CHECK_THROWS(connection.setMAC("ab-cd-ef-01-23-45")); // Wrong
        // separator
        CHECK_THROWS(connection.setMAC("ab?cd/ef-01?23/45")); // Wrong separator
        CHECK_THROWS(
            connection.setMAC("ab:cd.ef:01:23:45")); // Inconsistent separators
    }

    TEST_CASE("Incorrect Positioning of Separators")
    {
        CHECK_THROWS(
            connection.setMAC(":ab:cd:ef:01:23:45")); // Leading separator
        CHECK_THROWS(
            connection.setMAC("ab:cd:ef:01:23:45:")); // Trailing separator
        CHECK_THROWS(
            connection.setMAC("ab:cd:ef:01:2:345")); // Misplaced separator
    }

    TEST_CASE("Invalid Characters")
    {
        CHECK_THROWS(connection.setMAC(
            "ab:cd:ef:01:23:gh")); // Non-hexadecimal characters
        CHECK_THROWS(
            connection.setMAC("ab:cd:ef:01:23:45$")); // Special characters
    }

    TEST_CASE("Mixed Formats")
    {
        CHECK_THROWS(
            connection.setMAC("abcd.ef:01:2345")); // Mixing different formats
    }

    TEST_CASE("Multicast Address" * doctest::skip())
    {
        CHECK_THROWS(connection.setMAC(
            "01:00:5e:00:00:00")); // Multicast addresses are not typically used
    }

    TEST_CASE("Locally Administered Addresses" * doctest::skip())
    {
        CHECK_THROWS(connection.setMAC(
            "02:00:5e:00:00:00")); // Locally administered address
    }

    TEST_CASE("Zeroed Address" * doctest::skip())
    {
        CHECK_THROWS(connection.setMAC(
            "00:00:00:00:00:00")); // All zeros are not a valid hardware MAC
                                   // address
    }

    TEST_CASE("Broadcast Address" * doctest::skip())
    {
        CHECK_THROWS(connection.setMAC(
            "ff:ff:ff:ff:ff:ff")); // Reserved broadcast address
    }
}
