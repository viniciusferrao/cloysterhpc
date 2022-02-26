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
#include "services/log.h"

#include <iostream>
#include <regex>
#include <memory>

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

#ifdef _DEBUG_
#include <fmt/format.h>
#endif

// The rule of zero
//Cluster::Cluster() = default;
//Cluster::~Cluster() = default;

Headnode& Cluster::getHeadnode() {
    return m_headnode;
}

const std::string_view& Cluster::getName() const {
    return m_name;
}

void Cluster::setName(const std::string_view &name) {
    m_name = name;
}

const std::string_view& Cluster::getCompanyName() const {
    return m_companyName;
}

void Cluster::setCompanyName(const std::string_view &companyName) {
    m_companyName = companyName;
}

const std::string_view& Cluster::getAdminMail() const {
    return m_adminMail;
}

void Cluster::setAdminMail(const std::string_view &adminMail) {
    m_adminMail = adminMail;
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

Timezone& Cluster::getTimezone() {
    return m_timezone;
}

void Cluster::setTimezone(const std::string& tz) {
    m_timezone.setTimezone(tz);
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
 *  correctness
 */
void Cluster::setDomainName(const std::string& domainName) {
    if (domainName.size() > 255)
        throw;

#if __cpp_lib_starts_ends_with >= 201711L
    if (domainName.starts_with('-') or domainName.ends_with('-'))
#else
    if (boost::algorithm::starts_with(domainName, "-") or
        boost::algorithm::ends_with(domainName, "-"))
#endif
        throw std::runtime_error("Invalid domain name");

    /* Check if string has only digits */
    if (std::regex_match(domainName, std::regex("^[0-9]+$")))
        throw;

    /* Check if it's not only alphanumerics and - */
    if (!(std::regex_match(domainName, std::regex("^[A-Za-z0-9-.]+$"))))
        throw;

    m_domainName = domainName;
}

std::list<Network>& Cluster::getNetworks() {
    return m_network;
}

Network& Cluster::getNetwork(Network::Profile profile) {
    for (auto& network : m_network) {
        if (network.getProfile() == profile) {
            return network;
        }
    }
    throw std::runtime_error(fmt::format(
            "Cannot get any network with the profile {}",
            magic_enum::enum_name(profile)));
}

#if 0
const std::list<Network> Cluster::getNet(Network::Profile profile) {
    std::list<Network> network;

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

void Cluster::addNetwork() {
    m_network.emplace_back(Network());
}

void Cluster::addNetwork(Network::Profile profile) {
    m_network.emplace_back(Network(profile));
}

void Cluster::addNetwork(Network::Profile profile, Network::Type type) {
    m_network.emplace_back(Network(profile, type));
}

void Cluster::addNetwork(Network::Profile profile, Network::Type type,
                             const std::string& address,
                             const std::string& subnetMask,
                             const std::string& gateway,
                             const uint16_t& vlan,
                             const std::string& domainName,
                             const std::vector<std::string>& nameserver) {

    m_network.emplace_back(Network(
                profile, type, address, subnetMask, gateway, vlan, domainName,
                nameserver));
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

std::optional<std::unique_ptr<QueueSystem>>& Cluster::getQueueSystem() {
    return m_queueSystem;
}

void Cluster::setQueueSystem(QueueSystem::Kind kind) {
    switch (kind) {
        case QueueSystem::Kind::None:
            m_queueSystem = std::nullopt;
            break;

        case QueueSystem::Kind::SLURM:
            //m_queueSystem = std::unique_ptr<QueueSystem>(new SLURM());
            m_queueSystem = std::make_unique<SLURM>();
            break;

        case QueueSystem::Kind::PBS:
            //m_queueSystem = std::unique_ptr<QueueSystem>(new PBS());
            m_queueSystem = std::make_unique<PBS>();
            break;
    }
}

std::optional<Postfix>& Cluster::getMailSystem() {
    return m_mailSystem;
}

void Cluster::setMailSystem(Postfix::Profile profile) {
    m_mailSystem = Postfix(profile);
}

const std::filesystem::path& Cluster::getISOPath() const {
    return m_isoPath;
}

void Cluster::setISOPath(const std::filesystem::path& isoPath) {
    m_isoPath = isoPath;
}

const std::vector<Node>& Cluster::getNodes() const {
    return m_nodes;
}

void Cluster::addNode(std::string_view t_name, const Network& network, const std::string& t_address,
                      const std::string& t_mac, std::string_view t_bmcAddress,
                      std::string_view t_bmcUsername,
                      std::string_view t_bmcPassword) {
    m_nodes.emplace_back(t_name, network, t_address, t_mac, t_bmcAddress, t_bmcUsername,
                         t_bmcPassword);
}

#ifdef _DEBUG_
/* TODO: This debug function must be made as a template */
void Cluster::printNetworks(
        const std::list<Network>& networkType) {

    LOG_TRACE("Dump network data:");

#if __cplusplus < 202002L
    size_t i, j;
    for (auto const &network: networkType) {
        i = 0;
#else
    for (size_t i = 0; auto const &network: networkType) {
#endif
        std::cerr << fmt::format("Network [{}]", i++) << std::endl;
        std::cerr << "Address: " << network.getAddress() << std::endl;
        std::cerr << "Subnet Mask: " << network.getSubnetMask() << std::endl;
        std::cerr << "Gateway " << network.getGateway() << std::endl;
        std::cerr << "VLAN " << network.getVLAN() << std::endl;
        std::cerr << "Domain Name: " << network.getDomainName() << std::endl;
#if __cplusplus < 202002L
        j = 0;
        for (auto const &nameserver: network.getNameserver()) {
#else
        for (size_t j = 0; auto const &nameserver: network.getNameserver()) {
#endif
            std::cerr << fmt::format("Nameserver [{}]: {}\n", j++, nameserver);
        }
        std::cerr << "\n";
    }
}
#endif

#ifdef _DEBUG_
void Cluster::printData () {
    LOG_TRACE("Dump cluster data:");
    std::cerr << "Cluster attributes defined:" << std::endl;
    std::cerr << "Timezone: " << getTimezone().getTimezone() << std::endl;
    std::cerr << "Locale: " << getLocale() << std::endl;
    std::cerr << "Hostname: " << this->m_headnode.getHostname() << std::endl;
    std::cerr << "DomainName: " << getDomainName() << std::endl;
    std::cerr << "FQDN: " << this->m_headnode.getFQDN() << std::endl;

    printNetworks(m_network);

    std::cerr << "Provisioner: " << static_cast<int>(getProvisioner()) << std::endl;

    std::cerr << "xCATDynamicRangeStart: " << xCATDynamicRangeStart << std::endl;
    std::cerr << "xCATDynamicRangeEnd: " << xCATDynamicRangeEnd << std::endl;

    std::cerr << "Directory Admin Password: " << directoryAdminPassword << std::endl;
    std::cerr << "Directory Manager Password: " << directoryManagerPassword << std::endl;
    std::cerr << "Directory Disable DNSSEC: " << (directoryDisableDNSSEC ? "true" : "false") << std::endl;

    std::cerr << "nodePrefix: " << nodePrefix << std::endl;
    std::cerr << "nodePadding: " << nodePadding << std::endl;
    std::cerr << "nodeStartIP: " << nodeStartIP << std::endl;
    std::cerr << "nodeRootPassword: " << nodeRootPassword << std::endl;
    std::cerr << "nodeISOPath: " << getISOPath() << std::endl;

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
//    m_headnode.addConnection(m_network[0], "en0", "172.26.1.22");
//    m_headnode.addConnection(m_network[1], "en1", "10.1.1.7");
//    m_headnode.addConnection(m_network[2], "en1", "192.168.22.8");

    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);

    setISOPath("/root/OracleLinux-R8-U5-x86_64-dvd.iso");

    addNode("n01", getNetworks().front(), "192.168.0.1", "aa:bb:cc:11:22:33", "192.168.1.1",
            "ADMIN", "ADMIN");
    addNode("n02", getNetworks().front(), "192.168.0.2", "aa:ff:dc:22:da:cd", "192.168.1.2",
            "root", "calvin");

    /* Bad and old data */
    xCATDynamicRangeStart = "192.168.20.1";
    xCATDynamicRangeEnd = "192.168.20.254";
    directoryAdminPassword = "pwdAdmin";
    directoryManagerPassword = "pwdManager";
    directoryDisableDNSSEC = true;
    nodePrefix = "n";
    nodePadding = 2;
    nodeStartIP = "172.26.0.1";
    nodeRootPassword = "pwdNodeRoot";
    remoteAccess = true;
}

#endif
