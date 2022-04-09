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

const Headnode& Cluster::getHeadnode() const {
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

void Cluster::addNetwork(Network&& network) {
    m_network.emplace_back(network);
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
            m_queueSystem = std::make_unique<SLURM>(*this);
            break;

        case QueueSystem::Kind::PBS:
            //m_queueSystem = std::unique_ptr<QueueSystem>(new PBS());
            m_queueSystem = std::make_unique<PBS>(*this);
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

void Cluster::addNode(OS& t_os, std::string_view t_name,
                      const Network& network,
                      const std::string& t_address,
                      const std::string& t_mac) {
//    m_nodes.emplace_back(t_os, std::optional<BMC>(), t_name, network, t_address, t_mac);
    m_nodes.emplace_back(t_os, t_name, network, t_address, t_mac);
}

void Cluster::addNode(OS& t_os, std::string_view t_name,
                      const Network& network,
                      const std::string& t_address,
                      const std::string& t_mac, BMC& bmc) {
    m_nodes.emplace_back(t_os, t_name, network, t_address, t_mac, bmc);
}

#ifdef _DEBUG_
void Cluster::printNetworks(const std::list<Network>& networkType)
{

    LOG_TRACE("Dump network data:");

#if __cplusplus < 202002L
    size_t i, j;
    for (auto const &network : networkType) {
        i = 0;
#else
    for (size_t i = 0; auto const &network : networkType) {
#endif
        LOG_TRACE("Network [{}]", i++);
        LOG_TRACE("Profile: {}", magic_enum::enum_name(network.getProfile()));
        LOG_TRACE("Address: {}", network.getAddress());
        LOG_TRACE("Subnet Mask: {}", network.getSubnetMask());
        LOG_TRACE("Gateway: {}", network.getGateway());
        LOG_TRACE("VLAN: {}", network.getVLAN());
        LOG_TRACE("Domain Name: {}", network.getDomainName());
#if __cplusplus < 202002L
        j = 0;
        for (auto const &nameserver: network.getNameserver()) {
#else
        for (size_t j = 0; auto const &nameserver: network.getNameserver()) {
#endif
            LOG_TRACE("Nameserver [{}]: {}", j++, nameserver);
        }
    }
}
#endif

#ifndef _NDEBUG_
void Cluster::printData () {
    LOG_TRACE("Dump cluster data:");
    LOG_TRACE("Cluster attributes defined:");
    LOG_TRACE("OS Data:")
    m_headnode.getOS().printData();
    LOG_TRACE("Timezone: {}", getTimezone().getTimezone());
    LOG_TRACE("Locale: {}", getLocale());
    LOG_TRACE("Hostname: {}", this->m_headnode.getHostname());
    LOG_TRACE("DomainName: {}", getDomainName());
    LOG_TRACE("FQDN: {}", this->m_headnode.getFQDN());

    printNetworks(m_network);

    LOG_TRACE("Provisioner: {}", static_cast<int>(getProvisioner()));

    LOG_TRACE("xCATDynamicRangeStart: {}", xCATDynamicRangeStart);
    LOG_TRACE("xCATDynamicRangeEnd: {}", xCATDynamicRangeEnd);

    LOG_TRACE("Directory Admin Password: {}", directoryAdminPassword);
    LOG_TRACE("Directory Manager Password: {}", directoryManagerPassword);
    LOG_TRACE("Directory Disable DNSSEC: {}", (directoryDisableDNSSEC ? "true" : "false"));

    LOG_TRACE("nodePrefix: {}", nodePrefix);
    LOG_TRACE("nodePadding: {}", nodePadding);
    LOG_TRACE("nodeStartIP: {}", nodeStartIP);
    LOG_TRACE("nodeRootPassword: {}", nodeRootPassword);
    LOG_TRACE("nodeISOPath: {}", getISOPath().string());

    LOG_TRACE("Update system: {}", (isUpdateSystem() ? "true" : "false"));
    LOG_TRACE("Remote access: {}", (remoteAccess ? "true" : "false"));

    LOG_TRACE("Firewall: {}", (isFirewall() ? "true" : "false"));
    LOG_TRACE("SELinux: {}", static_cast<int>(getSELinux()));
}

void Cluster::fillTestData () {
    setFirewall(true);
    setSELinux(SELinuxMode::Disabled);
    setTimezone("America/Sao_Paulo");
    setLocale("en_US.UTF-8");
    this->m_headnode.setHostname(std::string_view{"headnode"});
    setDomainName("cluster.example.tld");
    this->m_headnode.setFQDN(
        fmt::format("{0}.{1}", this->m_headnode.getHostname(),
                    getDomainName()));
    setOFED(OFED::None);
    setQueueSystem(QueueSystem::Kind::SLURM);

    addNetwork(Network::Profile::External, Network::Type::Ethernet,
               "172.16.144.0", "255.255.255.0", "172.16.144.1", 0,
               "home.ferrao.net.br", { "172.16.144.1" });
    addNetwork(Network::Profile::Management, Network::Type::Ethernet,
               "172.26.0.0", "255.255.0.0", "0.0.0.0", 0, "cluster.example.tld",
               { "172.26.0.1" });
#if 0
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
#endif

    m_headnode.addConnection(getNetwork(Network::Profile::External),
                             "ens160", "172.16.144.50");
    m_headnode.addConnection(getNetwork(Network::Profile::Management),
                             "ens224", "172.26.255.254");
#if 0
    m_headnode.addConnection(getNetwork(Network::Profile::Service),
                             "ens224", "192.168.22.8");
#endif

    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);

    setISOPath("/root/OracleLinux-R8-U5-x86_64-dvd.iso");
    OS nodeOS(OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el8,
              OS::Distro::OL, "5.4.17-2136.302.6.1.el8uek.x86_64", 8, 5);
    addNode(nodeOS, "n01", getNetwork(Network::Profile::Management), "172.26.0.1", "00:0c:29:9b:0c:75");
    addNode(nodeOS, "n02", getNetwork(Network::Profile::Management), "172.26.0.2", "de:ad:be:ff:00:00");

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
