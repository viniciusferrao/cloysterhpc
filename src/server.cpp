/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "server.h"
#include "services/log.h"

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

// FIXME: A trigger to update FQDN should be made if hostname is changed
void Server::setHostname(const std::string& hostname)
{
    LOG_DEBUG("Running hostname checks against: {}", hostname);

    if (hostname.size() > 63)
        throw std::runtime_error(
            "Hostname cannot be bigger than 63 characters");

#if __cpp_lib_starts_ends_with >= 201711L
    if (hostname.starts_with('-') or hostname.ends_with('-'))
#else
    if (boost::algorithm::starts_with(hostname, "-")
        or boost::algorithm::ends_with(hostname, "-"))
#endif
        throw std::runtime_error("Invalid hostname");

    /* Check if string has only digits */
    if (std::regex_match(hostname, std::regex("^[0-9]+$")))
        throw std::runtime_error("Hostname cannot contain only digits");
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(hostname, std::regex("^[A-Za-z0-9-]+$"))))
        throw std::runtime_error("Hostname can only have alphanumerics");

    m_hostname = hostname;
}

// This overload is necessary for std::string_view compatibility
void Server::setHostname(std::string_view hostname)
{
    setHostname(std::string { hostname });
}

const std::string& Server::getFQDN() const noexcept { return m_fqdn; }

/* TODO: Validate if FQDN is in right format */
void Server::setFQDN(const std::string& fqdn)
{
    if (fqdn.size() > 255)
        throw std::runtime_error("FQDN cannot be bigger than 255 characters");

    m_fqdn = fqdn;
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
            magic_enum::enum_name(profile)));
}

const std::optional<BMC>& Server::getBMC() const { return m_bmc; }

void Server::setBMC(const BMC& bmc) { m_bmc = bmc; }

const CPU& Server::getCPU() const noexcept { return m_cpu; }

void Server::setCPU(const CPU& cpu) { m_cpu = cpu; }
