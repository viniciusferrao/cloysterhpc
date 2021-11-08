#include "connection.h"
#include "network.h"

#include <string>
#include <regex>

#include <arpa/inet.h> /* inet_*() functions */
#include <ifaddrs.h> /* getifaddrs() */

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

Connection::Connection() = default;

Connection::~Connection() = default;

Connection::Connection(const std::string& interface,
                       const std::string& address) {
    setInterface(interface);
    setAddress(address);
}

Connection::Connection(std::shared_ptr<Network> network,
                       const std::string& interface,
                       const std::string& address)
                       : m_network(network) {

    setInterface(interface);
    setAddress(address);
}

Connection::Connection(const std::string& interface, const std::string& address,
                       const std::string& hostname, const std::string& fqdn) {

    setInterface(interface);
    setAddress(address);
    setHostname(hostname);
    setFQDN(fqdn);
}

const std::string Connection::getInterface() const {
    return m_interface;
}

void Connection::setInterface (const std::string& interface) {
    if (interface == "lo")
        throw; /* Cannot use the loopback interface */

    /* TODO: Use smart pointers */
    /* Code based on getifaddrs(3) man page */
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw; /* TODO: parse errno to throw */

    for (ifa = ifaddr; ifa != nullptr ; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        if (interface == ifa->ifa_name) {
            m_interface = interface;
            freeifaddrs(ifaddr);
            return;
        }
    }

    freeifaddrs(ifaddr);
    throw; /* Interface not found */
}

const std::string Connection::getAddress () const {
    if (inet_ntoa(m_address) == nullptr)
        throw;
    return inet_ntoa(m_address);
}

void Connection::setAddress (const std::string& address) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        throw; //return -1; /* Invalid IP Address */
}

const std::string& Connection::getHostname() const {
    return m_hostname;
}

void Connection::setHostname(const std::string& hostname) {
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

const std::string& Connection::getFQDN() const {
    return m_fqdn;
}

void Connection::setFQDN(const std::string& fqdn) {
    if (fqdn.size() > 255)
        throw;

    m_fqdn = fqdn;
}

const std::shared_ptr<Network>& Connection::getNetwork() const {
    return m_network;
}

void Connection::setNetwork(std::shared_ptr<Network> network) {
    m_network = network;
}
