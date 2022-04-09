#include "server.h"

const OS& Server::getOS() const noexcept {
    return m_os;
}

void Server::setOS(const OS& os) {
    m_os = os;
}

const std::string& Server::getHostname() const noexcept {
    return m_hostname;
}

void Server::setHostname(const std::string& hostname) {
    if (hostname.size() > 63)
        throw;

#if __cpp_lib_starts_ends_with >= 201711L
    if (hostname.starts_with('-') or hostname.ends_with('-'))
#else
        if (boost::algorithm::starts_with(hostname, "-") or
        boost::algorithm::ends_with(hostname, "-"))
#endif
        throw std::runtime_error("Invalid hostname");

    /* Check if string has only digits */
    if (std::regex_match(hostname, std::regex("^[0-9]+$")))
        throw;
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(hostname, std::regex("^[A-Za-z0-9-]+$"))))
        throw;

    m_hostname = hostname;
}

void Server::setHostname(std::string_view hostname) {
    setHostname(std::string{hostname});
}

const std::string& Server::getFQDN() const noexcept {
    return m_fqdn;
}

/* TODO: Validate if FQDN is in right format */
void Server::setFQDN(const std::string& fqdn) {
    if (fqdn.size() > 255)
        throw;

    m_fqdn = fqdn;
}

//const std::unique_ptr<Connection>& Server::getConnection() const {
//    return m_externalConnection;
//}

const std::list<Connection>& Server::getConnections() const {
    return m_connection;
}

void Server::addConnection(const Network& network) {
    m_connection.emplace_back(network);
}

void Server::addConnection(const Network& network,
                             const std::string& interface,
                             const std::string& address) {

    m_connection.emplace_back(network, interface, address);
}

void Server::addConnection(Connection&& connection) {
    m_connection.emplace_back(connection);
}

//const Connection& Server::getConnection(Network::Profile profile) const {
//    for (auto const& connection : std::as_const(m_connection)) {
//        if (connection.getNetwork().getProfile() == profile)
//            return connection;
//    }
//    throw; /* Cannot find a connection with profile */
//}

Connection& Server::getConnection(Network::Profile profile) {
//    auto it = std::find_if(
//            m_connection.begin(), m_connection.end(),
//            [&](const Connection& x){
//        return x.getNetwork().getProfile() == profile;
//    });
//
//    return *it;

    for (auto& connection : m_connection) {
        if (connection.getNetwork().getProfile() == profile) {
            return connection;
        }
    }

    throw std::runtime_error(fmt::format(
            "Cannot get any connection with profile {}",
            magic_enum::enum_name(profile)));
}

const std::optional<BMC>& Server::getBMC() const {
    return m_bmc;
}
