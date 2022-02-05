#include "network.h"
#include "services/log.h"

#include <vector>
#include <string>
#include <regex>
#include <arpa/inet.h> /* inet_*() functions */

#if __cplusplus < 202002L
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
    setNameserver(nameserver);
}

Network::~Network() = default;

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
const std::string Network::getAddress() const {
    if (inet_ntoa(m_address) == nullptr)
        throw;
    return inet_ntoa(m_address);
}

void Network::setAddress(const std::string& address) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        throw; //return -1; /* Invalid IP Address */
}

const std::string Network::getSubnetMask() const {
    if (inet_ntoa(m_subnetMask) == nullptr)
        throw;
    return inet_ntoa(m_subnetMask);
}

void Network::setSubnetMask(const std::string& subnetMask) {
    if (inet_aton(subnetMask.c_str(), &this->m_subnetMask) == 0)
        throw; //return -1; /* Invalid IP Address */
}

const std::string Network::getGateway() const {
    if (inet_ntoa(m_gateway) == nullptr)
        throw;
    return inet_ntoa(m_gateway);
}

void Network::setGateway(const std::string& gateway) {
    if (inet_aton(gateway.c_str(), &this->m_gateway) == 0)
        throw; //return -1; /* Invalid IP Address */
}
/* End of TODO */

const uint16_t& Network::getVLAN() const {
    return m_vlan;
}

void Network::setVLAN(const uint16_t& vlan) {
    if (vlan >= 4096)
        throw;
    m_vlan = vlan;
}

const std::string &Network::getDomainName() const {
    return m_domainName;
}

void Network::setDomainName(const std::string& domainName) {
    if (domainName.size() > 255)
        throw;

#if __cplusplus >= 202002L
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

/* TODO: Check return type
 *  - We can't return const (don't know exactly why)
 */
std::vector<std::string> Network::getNameserver() const {
    std::vector<std::string> returnVector;
    for (auto const& ns : std::as_const(m_nameserver))
        returnVector.emplace_back(inet_ntoa(ns));

    return returnVector;
}

/* TODO: Test this */
void Network::setNameserver(const std::vector<std::string>& nameserver) {
    if (nameserver.empty())
        return;

    m_nameserver.reserve(nameserver.size());
    struct in_addr aux {};

#if __cplusplus < 202002L
    size_t i = 0;
    for (auto const& ns : std::as_const(nameserver)) {
#else
    for (size_t i = 0 ; auto const& ns : std::as_const(nameserver)) {
#endif
        m_nameserver.push_back(aux); /* aux may have garbage on it */
        if (inet_aton(ns.c_str(), &this->m_nameserver[i++]) == 0)
            throw;
    }
}

#ifndef _NDEBUG_
void Network::dumpNetwork() const {
    LOG_DEBUG("Profile: {}", getProfileString.at(m_profile));
    LOG_DEBUG("Type: {}", getTypeString.at(m_type));
}
#endif
