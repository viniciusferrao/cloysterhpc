/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "cluster.h"
#include "functions.h"
#include "headnode.h"
#include "services/xcat.h"

#include <iostream>
#include <regex>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

#ifdef _DEBUG_
#include <fmt/format.h>
#endif

Cluster::Cluster () {
    m_headnode = std::make_unique<Headnode>();
}

Cluster::~Cluster () = default;

bool Cluster::isFirewall() const {
    return m_firewall;
}

void Cluster::setFirewall(bool firewall) {
    m_firewall = firewall;
}

bool Cluster::isSELinux() const {
    return m_selinux;
}

void Cluster::setSELinux(bool selinux) {
    m_selinux = selinux;
}

const std::string& Cluster::getTimezone() const {
    return m_timezone;
}

void Cluster::setTimezone(const std::string& timezone) {
    m_timezone = timezone;
}

const std::string& Cluster::getLocale() const {
    return m_locale;
}

void Cluster::setLocale(const std::string &locale) {
    m_locale = locale;
}

const std::string& Cluster::getDomainName() const {
    return m_domainName;
}

/* TODO: Fix logic, split domain to a vector after each dot (.) to check for
 * correctness
 */
void Cluster::setDomainName(const std::string& domainName) {
    if (domainName.size() > 255)
        throw;

#if __cplusplus >= 202002L
    if (domainName.starts_with('-') or domainName.ends_with('-'))
        throw;
#else
    if (boost::algorithm::starts_with(domainName, '-') or
        boost::algorithm::ends_with(domainName, '-'));
#endif

    /* Check if string has only digits */
    if (std::regex_match(domainName, std::regex("^[0-9]+$")))
        throw;

    /* Check if it's not only alphanumerics and - */
    if (!(std::regex_match(domainName, std::regex("^[A-Za-z0-9-.]+$"))))
        throw;

    m_domainName = domainName;
}

/* TODO: Fix this implementation of network; this is just dumb */
const Network Cluster::getNetwork(Network::Profile profile) const {
    switch (profile) {
        case Network::Profile::External:
            return m_network.external;
        case Network::Profile::Management:
            return m_network.management;
        case Network::Profile::Service:
            return m_network.service;
        case Network::Profile::Application:
            return m_network.application;
    }
}

void Cluster::setNetwork(Network::Profile profile, Network::Type type,
                         std::string address, std::string subnetMask,
                         std::string gateway, uint16_t vlan,
                         std::string domainName,
                         std::vector<std::string> nameserver) {

    Network network(profile, type);
    network.setAddress(address);
    network.setSubnetMask(subnetMask);
    network.setGateway(gateway);
    network.setVLAN(vlan);
    network.setDomainName(domainName);
    network.setNameserver(nameserver);
}

#ifdef _DEBUG_
void Cluster::printData () {
    std::cerr << "Cluster attributes defined:" << std::endl;
    std::cerr << "Timezone: " << getTimezone() << std::endl;
    std::cerr << "Locale: " << getLocale() << std::endl;
    std::cerr << "Hostname: " << this->m_headnode->getHostname() << std::endl;
    std::cerr << "Domainname: " << getDomainName() << std::endl;
    std::cerr << "FQDN: " << this->m_headnode->getFQDN() << std::endl;
    //std::cerr << "interfaceExternal: " << cluster.interfaceExternal << std::endl;
    //std::cerr << "interfaceInternal: " << cluster.interfaceInternal << std::endl;
    //std::cerr << "interfaceInternalNetwork: " << cluster.interfaceInternalNetwork << std::endl;
    //std::cerr << "interfaceInternalIP: " << cluster.interfaceInternalIP << std::endl;
    std::cerr << "xCATDynamicRangeStart: " << xCATDynamicRangeStart << std::endl;
    std::cerr << "xCATDynamicRangeEnd: " << xCATDynamicRangeEnd << std::endl;

    std::cerr << "Directory Admin Password: " << directoryAdminPassword << std::endl;
    std::cerr << "Directory Manager Password: " << directoryManagerPassword << std::endl;
    std::cerr << "Directory Disable DNSSEC: " << (directoryDisableDNSSEC ? "true" : "false") << std::endl;

    std::cerr << "nodePrefix: " << nodePrefix << std::endl;
    std::cerr << "nodePadding: " << nodePadding << std::endl;
    std::cerr << "nodeStartIP: " << nodeStartIP << std::endl;
    std::cerr << "nodeRootPassword: " << nodeRootPassword << std::endl;
    std::cerr << "nodeISOPath: " << nodeISOPath << std::endl;

    std::cerr << "ibStack: " << ibStack << std::endl;

    std::cerr << "queueSystem: " << queueSystem.name << std::endl;
    // if (queueSystem.name == "SLURM")
    //     std::cerr << "slurm.partition: " << queueSystem.slurm.partition << std::endl;
    // if (queueSystem.name == "PBS")
    //     std::cerr << "pbs.defaultPlace: " << queueSystem.pbs.defaultPlace << std::endl;

    // std::cerr << "Enable Postfix: " << postfix.enable ? "true" : "false" << std::endl;
    // if (postfix.enable) {
    //     std::cerr << "\t-> Profile: " << postfixProfiles[cluster.postfix.profileId] << std::endl;
    //     switch (cluster.postfix.profileId) {
    //         case 0:
    //             /* Local */
    //             break;
    //         case 1:
    //             /* Relay */
    //             std::cerr << "\t\t-> Hostname: " << cluster.postfix.relay.m_hostname << std::endl;
    //             std::cerr << "\t\t-> Port: %u\n", cluster.postfix.relay.port << std::endl;
    //             break;
    //         case 2:
    //             /* SASL */
    //             std::cerr << "\t\t-> Hostname: " << cluster.postfix.sasl.m_hostname << std::endl;
    //             std::cerr << "\t\t-> Port: %u\n", cluster.postfix.sasl.port << std::endl;
    //             std::cerr << "\t\t-> Username: " << cluster.postfix.sasl.username << std::endl;
    //             std::cerr << "\t\t-> Password: " << cluster.postfix.sasl.password << std::endl;
    //             break;
    //     }
    // }

    std::cerr << "Update system: " << (updateSystem ? "true" : "false") << std::endl;
    std::cerr << "Remote access: " << (remoteAccess ? "true" : "false") << std::endl;

    std::cerr << "Firewall: " << (isFirewall() ? "true" : "false") << std::endl;
    std::cerr << "SELinux: " << (isSELinux() ? "true" : "false") << std::endl;
}

void Cluster::fillTestData () {
    setFirewall(true);
    setSELinux(true);
    setTimezone("America/Sao_Paulo");
    setLocale("en_US.UTF-8");
    this->m_headnode->setHostname("headnode");
    setDomainName("cluster.example.tld");
    this->m_headnode->setFQDN(
        fmt::format("{0}.{1}", this->m_headnode->getHostname(),
                    getDomainName()));

    xCATDynamicRangeStart = "192.168.20.1";
    xCATDynamicRangeEnd = "192.168.20.254";
    directoryAdminPassword = "pwdAdmin";
    directoryManagerPassword = "pwdManager";
    directoryDisableDNSSEC = true;
    nodePrefix = "n";
    nodePadding = "2";
    nodeStartIP = "172.26.0.1";
    nodeRootPassword = "pwdNodeRoot";
    nodeISOPath = "/mnt/iso/rhel-8.4-dvd.iso";
    ibStack = "MLNX";
    queueSystem.name = "SLURM";
    updateSystem = true;
    remoteAccess = true;
}

#endif
