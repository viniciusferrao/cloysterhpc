#include "connection.h"
#include "network.h"

#include <string>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <utility>

#include <arpa/inet.h> /* inet_*() functions */
#include <ifaddrs.h> /* getifaddrs() */
#include <cstring>

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

Connection::Connection(const Network& network,
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
        throw std::runtime_error("Cannot use the loopback interface");

    /* TODO: Use smart pointers */
    /* Code based on getifaddrs(3) man page */
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        throw; /* TODO: parse errno to throw */

    for (ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        // TODO: Since we are already here, get the MAC Address from sa_data
        if (interface == ifa->ifa_name) {
            m_interface = interface;

            freeifaddrs(ifaddr);
            return;
        }
    }

    freeifaddrs(ifaddr);

    /* Interface not found, bugged */
    throw std::runtime_error("Cannot find network interface");
}

std::vector<std::string> Connection::fetchInterfaces() const {
    struct ifaddrs *ifaddr, *ifa;
    std::vector<std::string> interfaces;

    if (getifaddrs(&ifaddr) == -1)
        throw; /* TODO: parse errno to throw */

    for (ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next) {
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

const std::string& Connection::getMAC() const {
    return m_mac;
}

void Connection::setMAC(const std::string& mac) {
    if ((mac.size() != 12) && (mac.size() != 14) && (mac.size() != 17))
        throw 1;

    // TODO: Make it easier to read and consider the Cisco MAC identifier
    const std::regex pattern(
            "^([0-9A-Fa-f]{2}[:-]){5}"
            "([0-9A-Fa-f]{2})|([0-9a-"
            "fA-F]{4}\\.[0-9a-fA-F]"
            "{4}\\.[0-9a-fA-F]{4})$");

    if (regex_match(mac, pattern))
        m_mac = boost::algorithm::to_lower_copy(mac);
    else
        throw "invalid";
}

const std::string Connection::getAddress () const {
    if (inet_ntoa(m_address) == nullptr)
        throw; // Invalid IP
    return inet_ntoa(m_address);
}

void Connection::setAddress (const std::string& address) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        throw "XXX"; //return -1; /* Invalid IP Address */
}

const std::string Connection::fetchAddress()
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
        // TODO: Check for errno
        throw std::runtime_error("Error fetching network interfaces");

    for (ifa = ifaddr ; ifa != nullptr ; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        // TODO: Test this
        if (std::strcmp(ifa->ifa_name, getInterface().c_str()) == 0)
            std::memcpy(&m_address, ifa->ifa_addr, sizeof(&ifa->ifa_addr));
    }

    return getAddress();
}


const std::string& Connection::getHostname() const {
    return m_hostname;
}

void Connection::setHostname(const std::string& hostname) {
    if (hostname.size() > 63)
        throw std::range_error(
                "Hostname cannot be bigger than 64 characters");

#if __cplusplus >= 202002L
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

const std::string& Connection::getFQDN() const {
    return m_fqdn;
}

void Connection::setFQDN(const std::string& fqdn) {
    if (fqdn.size() > 255)
        throw;

    m_fqdn = fqdn;
}

const Network& Connection::getNetwork() const {
    return m_network;
}

//void Connection::setNetwork(const Network& network) {
//    m_network = std::move(network);
//}
