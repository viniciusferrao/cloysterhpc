/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include "cluster.h"
#include "functions.h"
#include "headnode.h"
#include "services/xcat.h"

#include <iostream>
#include <regex>
#include <memory>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

#ifdef _DEBUG_
#include <fmt/format.h>
#endif

Cluster::Cluster () {
    //m_headnode = std::make_unique<Headnode>();
}

Cluster::~Cluster () = default;

const Headnode Cluster::getHeadnode() const {
//const std::unique_ptr<Headnode>& Cluster::getHeadnode() const {
    return m_headnode;
}

bool Cluster::isFirewall() const {
    return m_firewall;
}

void Cluster::setFirewall(bool firewall) {
    m_firewall = firewall;
}

Cluster::SELinuxMode Cluster::getSELinux() const {
    return m_selinux;
}

void Cluster::setSELinux(Cluster::SELinuxMode mode) {
    m_selinux = mode;
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

/* TODO: This implementation of network is not that great */
const std::vector<std::shared_ptr<Network>>& Cluster::getNetwork(
                                        Network::Profile profile) const {

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

#if 0
const std::vector<Network> Cluster::getNet(Network::Profile profile) {
    std::vector<Network> network;

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
#endif

void Cluster::addNetwork(Network::Profile profile, Network::Type type,
                             const std::string& address,
                             const std::string& subnetMask,
                             const std::string& gateway,
                             const uint16_t& vlan,
                             const std::string& domainName,
                             const std::vector<std::string>& nameserver) {

    switch (profile) {
        case Network::Profile::External:
            m_network.external.emplace_back(make_shared<Network>(
                profile, type, address, subnetMask, gateway, vlan, domainName,
                nameserver));
            break;

        case Network::Profile::Management:
            m_network.management.emplace_back(make_shared<Network>(
                profile, type, address, subnetMask, gateway, vlan, domainName,
                nameserver));
            break;

        case Network::Profile::Service:
            m_network.service.emplace_back(make_shared<Network>(
                profile, type, address, subnetMask, gateway, vlan, domainName,
                nameserver));
            break;

        case Network::Profile::Application:
            m_network.application.emplace_back(make_shared<Network>(
                profile, type, address, subnetMask, gateway, vlan, domainName,
                nameserver));
            break;
    }
}

bool Cluster::isUpdateSystem() const {
    return m_updateSystem;
}

void Cluster::setUpdateSystem(bool updateSystem) {
    m_updateSystem = updateSystem;
}

Cluster::Provisioner Cluster::getProvisioner() const {
    return m_provisioner;
}

void Cluster::setProvisioner(Cluster::Provisioner provisioner) {
    m_provisioner = provisioner;
}

Cluster::OFED Cluster::getOFED() const {
    return m_ofed;
}

void Cluster::setOFED(Cluster::OFED ofed) {
    m_ofed = ofed;
}

const std::vector<Node>& Cluster::getNodes() const {
    return m_nodes;
}

void Cluster::addNode(std::string_view t_name, const std::string& t_address,
                      const std::string& t_mac, std::string_view t_bmcAddress,
                      std::string_view t_bmcUsername,
                      std::string_view t_bmcPassword) {
    m_nodes.emplace_back(t_name, t_address, t_mac, t_bmcAddress, t_bmcUsername,
                         t_bmcPassword);
}

#ifdef _DEBUG_
/* TODO: This debug function must be made as a template */
void Cluster::printNetworks() {
    for (size_t i = 0 ; auto const& network : m_network.external) {
        std::cerr << fmt::format("External Network [{}]", i++) << std::endl;
        std::cerr << "Address: " << network->getAddress() << std::endl;
        std::cerr << "Subnet Mask: " << network->getSubnetMask() << std::endl;
        std::cerr << "Gateway " << network->getGateway() << std::endl;
        std::cerr << "VLAN " << network->getVLAN() << std::endl;
        std::cerr << "Domain Name: " << network->getDomainName() << std::endl;
        for (size_t j = 0 ; auto const& nameserver : network->getNameserver()) {
            std::cerr << fmt::format("Nameserver [{}]: {}\n", j++, nameserver);
        }
    }
    std::cerr << std::endl;

    for (size_t i = 0 ; auto const& network : m_network.management) {
        std::cerr << fmt::format("Management Network [{}]", i++) << std::endl;
        std::cerr << "Address: " << network->getAddress() << std::endl;
        std::cerr << "Subnet Mask: " << network->getSubnetMask() << std::endl;
        std::cerr << "Gateway " << network->getGateway() << std::endl;
        std::cerr << "VLAN " << network->getVLAN() << std::endl;
        std::cerr << "Domain Name: " << network->getDomainName() << std::endl;
        for (size_t j = 0 ; auto const& nameserver : network->getNameserver()) {
            std::cerr << fmt::format("Nameserver [{}]: {}\n", j++, nameserver);
        }
    }
    std::cerr << std::endl;

    for (size_t i = 0 ; auto const& network : m_network.service) {
        std::cerr << fmt::format("Service Network [{}]", i++) << std::endl;
        std::cerr << "Address: " << network->getAddress() << std::endl;
        std::cerr << "Subnet Mask: " << network->getSubnetMask() << std::endl;
        std::cerr << "Gateway " << network->getGateway() << std::endl;
        std::cerr << "VLAN " << network->getVLAN() << std::endl;
        std::cerr << "Domain Name: " << network->getDomainName() << std::endl;
        for (size_t j = 0 ; auto const& nameserver : network->getNameserver()) {
            std::cerr << fmt::format("Nameserver [{}]: {}\n", j++, nameserver);
        }
    }
    std::cerr << std::endl;

    for (size_t i = 0 ; auto const& network : m_network.application) {
        std::cerr << fmt::format("Application Network [{}]", i++) << std::endl;
        std::cerr << "Address: " << network->getAddress() << std::endl;
        std::cerr << "Subnet Mask: " << network->getSubnetMask() << std::endl;
        std::cerr << "Gateway " << network->getGateway() << std::endl;
        std::cerr << "VLAN " << network->getVLAN() << std::endl;
        std::cerr << "Domain Name: " << network->getDomainName() << std::endl;
        for (size_t j = 0 ; auto const& nameserver : network->getNameserver()) {
            std::cerr << fmt::format("Nameserver [{}]: {}\n", j++, nameserver);
        }
    }
    std::cerr << std::endl;

}
#endif

#ifdef _DEBUG_
void Cluster::printData () {
    std::cerr << "Cluster attributes defined:" << std::endl;
    std::cerr << "Timezone: " << getTimezone() << std::endl;
    std::cerr << "Locale: " << getLocale() << std::endl;
    std::cerr << "Hostname: " << this->m_headnode.getHostname() << std::endl;
    std::cerr << "DomainName: " << getDomainName() << std::endl;
    std::cerr << "FQDN: " << this->m_headnode.getFQDN() << std::endl;

    printNetworks();

    std::cerr << "Provisioner: " << (int)getProvisioner() << std::endl;

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

    std::cerr << "Update system: " << (isUpdateSystem() ? "true" : "false") << std::endl;
    std::cerr << "Remote access: " << (remoteAccess ? "true" : "false") << std::endl;

    std::cerr << "Firewall: " << (isFirewall() ? "true" : "false") << std::endl;
    std::cerr << "SELinux: " << static_cast<int>(getSELinux()) << std::endl;
}

void Cluster::fillTestData () {
    setFirewall(true);
    setSELinux(SELinuxMode::Disabled);
    setTimezone("America/Sao_Paulo");
    setLocale("en_US.UTF-8");
    this->m_headnode.setHostname("headnode");
    setDomainName("cluster.example.tld");
    this->m_headnode.setFQDN(
        fmt::format("{0}.{1}", this->m_headnode.getHostname(),
                    getDomainName()));
    setOFED(OFED::Inbox);

    addNetwork(Network::Profile::External, Network::Type::Ethernet,
               "192.2.0.0", "255.255.255.192", "192.2.0.1", 0, "example.com",
               { "1.1.1.1", "208.67.222.222" });
    addNetwork(Network::Profile::Management, Network::Type::Ethernet,
               "10.0.0.0", "255.0.0.0", "0.0.0.0", 0, "cluster.example.com",
               { "10.255.255.254" });
    addNetwork(Network::Profile::Service, Network::Type::Ethernet,
               "172.16.0.0", "255.255.0.0", "0.0.0.0", 0,
               "srv.cluster.example.com", { "172.16.255.254" });
    addNetwork(Network::Profile::Service, Network::Type::Ethernet,
               "172.24.0.0", "255.255.0.0", "0.0.0.0", 0,
               "srv2.cluster.example.com", { "1.1.1.1", "172.24.0.1" });
    addNetwork(Network::Profile::Application, Network::Type::Infiniband,
               "192.168.0.0", "255.255.255.0", "0.0.0.0", 0,
               "ib.cluster.example.com", { "0.0.0.0" });
    addNetwork(Network::Profile::Application, Network::Type::Infiniband,
               "192.168.1.0", "255.255.255.0", "0.0.0.0", 4094,
               "ib2.cluster.example.com", { "0.0.0.0" });

//    m_headnode.getConnection().front().setNetwork(
//            getNetwork(Network::Profile::External).front());
//    m_headnode.getConnection().front().setNetwork(m_network.external.front());
    m_headnode.addConnection(m_network.external.front(), "en0", "172.26.1.22");
    m_headnode.addConnection(m_network.management.front(), "en1", "10.1.1.7");

    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);

    addNode("n01", "192.168.0.1", "aa:bb:cc:11:22:33", "192.168.1.1",
            "ADMIN", "ADMIN");
    addNode("n02", "192.168.0.2", "aa:ff:dc:22:da:cd", "192.168.1.2",
            "root", "calvin");

    /* Bad and old data */
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
    remoteAccess = true;
}

#endif
