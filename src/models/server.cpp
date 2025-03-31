/*
 * o
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/server.h>
#include <cloysterhpc/services/log.h>
#include <regex>
#include <string_view>

namespace cloyster::models {
Server::Server(std::string_view hostname, OS& os, CPU& cpu,
    std::list<Connection>&& connections, std::optional<BMC> bmc)
    : m_os(os)
    , m_cpu(cpu)
    , m_connection(connections)
    , m_bmc(bmc)
{

    setHostname(hostname);
}

const OS& Server::getOS() const noexcept { return m_os; }

void Server::setOS(const OS& os) { m_os = os; }

const std::string& Server::getHostname() const noexcept { return m_hostname; }

/**
 * @brief Validate the hostname
 *
 * Return the hostname if is valid, or an error message
 */
std::expected<std::string_view, std::string> Server::validateHostname()
{
    LOG_DEBUG("Running hostname checks against: {}", m_hostname);

    if (m_hostname.size() > 63)
        throw std::string { "Hostname cannot be bigger than 63 characters" };

    if (m_hostname.starts_with('-') or m_hostname.ends_with('-'))
        throw std::string { "Invalid hostname" };

    /* Check if string has only digits */
    if (std::regex_match(m_hostname, std::regex("^[0-9]+$")))
        throw std::string { "Hostname cannot contain only digits" };
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(m_hostname, std::regex("^[A-Za-z0-9-]+$"))))
        return std::string { "Hostname can only have alphanumerics" };

    return std::string_view { m_hostname };
}

// FIXME: A trigger to update FQDN should be made if hostname is changed
void Server::setHostname(const std::string& hostname) { m_hostname = hostname; }

// This overload is necessary for std::string_view compatibility
void Server::setHostname(std::string_view hostname)
{
    setHostname(std::string { hostname });
}

const std::string& Server::getFQDN() const noexcept { return m_fqdn; }

void Server::setFQDN(const std::string& fqdn) { m_fqdn = fqdn; }

std::expected<std::string_view, std::string> Server::validateFQDN()
{
    if (m_fqdn.size() > 255)
        return std::string("FQDN cannot be bigger than 255 characters");

    // This pattern validates whether an FQDN is valid or not.
    const std::regex fqdnPattern(
        R"regex(^(?:[a-zA-Z0-9](?:[a-zA-Z0-9\\-]*[a-zA-Z0-9])?\.)+[A-Za-z0-9](?:[A-Za-z0-9\\-]*[A-Za-z0-9])?$)regex");

    if (!std::regex_match(m_fqdn, fqdnPattern))
        return std::string("Invalid FQDN format");

    return std::string_view { m_fqdn };
}

const std::list<Connection>& Server::getConnections() const
{
    return m_connection;
}

void Server::addConnection(Network& network)
{
    m_connection.emplace_back(&network);
}

void Server::addConnection(Network& network,
    std::optional<std::string_view> interface, std::string_view mac,
    const std::string& ip)
{
    m_connection.emplace_back(&network, interface, mac, ip);
}

void Server::addConnection(Connection&& connection)
{
    m_connection.emplace_back(connection);
}

// const Connection& Server::getConnection(Network::Profile profile) const {
//     for (const auto& connection : std::as_const(m_connection)) {
//         if (connection.getNetwork().getProfile() == profile)
//             return connection;
//     }
//     throw; /* Cannot find a connection with profile */
// }

const Connection& Server::getConnection(Network::Profile profile) const
{
    for (const auto& connection : m_connection) {
        if (connection.getNetwork()->getProfile() == profile) {
            return connection;
        }
    }

    throw std::runtime_error(
        fmt::format("Cannot get any connection with profile {}",
            cloyster::utils::enums::toString(profile)));
}

void Server::setConnection(const std::list<Connection>& connection)
{
    m_connection = connection;
}

const std::optional<BMC>& Server::getBMC() const { return m_bmc; }

void Server::setBMC(const BMC& bmc) { m_bmc = bmc; }

const CPU& Server::getCPU() const noexcept { return m_cpu; }

void Server::setCPU(const CPU& cpu) { m_cpu = cpu; }

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

TEST_SUITE("Test FQDN")
{
    TEST_CASE("FQDN Validation with Server::setFQDN" * doctest::skip())
    {
        Server server;

        SUBCASE("Valid FQDNs")
        {
            CHECK_NOTHROW(server.setFQDN("example.com"));
            CHECK(server.validateFQDN().has_value());
            CHECK_NOTHROW(server.setFQDN("subdomain.example.com"));
            CHECK(server.validateFQDN().has_value());
            CHECK_NOTHROW(server.setFQDN("sub-domain.example.co.uk"));
        }

        SUBCASE("Invalid FQDNs")
        {
            CHECK_THROWS(server.setFQDN("example")); // Missing TLD
            CHECK(!server.validateFQDN().has_value());
            CHECK_THROWS(server.setFQDN(".example.com")); // Leading dot
            CHECK(!server.validateFQDN().has_value());
            CHECK_THROWS(server.setFQDN("example.com.")); // Trailing dot
            CHECK(!server.validateFQDN().has_value());
            CHECK_THROWS(server.setFQDN("example..com")); // Double dot
            CHECK(!server.validateFQDN().has_value());
            CHECK_THROWS(server.setFQDN("example@com")); // Invalid character
            CHECK(!server.validateFQDN().has_value());
            CHECK_THROWS(
                server.setFQDN(std::string(256, 'a'))); // FQDN too long
            CHECK(!server.validateFQDN().has_value());
        }
    }
}

}
