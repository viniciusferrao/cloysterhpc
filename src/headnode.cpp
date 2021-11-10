#include "headnode.h"

#include <iostream>
#include <fstream>
#include <algorithm> /* std::remove */
#include <map> /* std::map */
#include <regex>

#include <sys/utsname.h>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

/* The constructor should discover everything we need from the machine that is
 * running the software. We always consider that the software runs from the
 * server that will become the cluster headnode.
 */
//Headnode::Headnode () = default;

Headnode::Headnode()
    : m_hostname(discoverHostname()) {}

const std::string &Headnode::getHostname() const {
    return m_hostname;
}

void Headnode::setHostname(const std::string &hostname) {
    if (hostname.size() > 63)
        throw;

#if __cplusplus >= 202002L
    if (hostname.starts_with('-') or hostname.ends_with('-'))
#else
    if (boost::algorithm::starts_with(hostname, '-') or
        boost::algorithm::ends_with(hostname, '-'));
#endif
        throw;

    /* Check if string has only digits */
    if (std::regex_match(hostname, std::regex("^[0-9]+$")))
        throw;
    /* Check if string is not only alphanumerics and - */
    if (!(std::regex_match(hostname, std::regex("^[A-Za-z0-9-]+$"))))
        throw;
    
    m_hostname = hostname;
}

const std::string Headnode::discoverHostname() {
    struct utsname system {};

    uname(&system);
    std::string_view hostname = system.nodename;
    m_hostname = hostname.substr(0, hostname.find('.'));

    return m_hostname;
}

const std::string &Headnode::getFQDN() const {
    return m_fqdn;
}

void Headnode::setFQDN(const std::string &fqdn) {
    if (fqdn.size() > 255)
        throw;

    m_fqdn = fqdn;
}

const OS &Headnode::getOS() const {
    return m_os;
}

void Headnode::setOS(const OS &os) {
    m_os = os;
}

//const std::unique_ptr<Connection>& Headnode::getConnection() const {
//    return m_externalConnection;
//}

const std::vector<Connection>& Headnode::getConnections() const {
    return m_connection;
}

void Headnode::addConnection(const std::shared_ptr<Network>& network,
                             const std::string& interface,
                             const std::string& address) {

    m_connection.emplace_back(network, interface, address);
}
