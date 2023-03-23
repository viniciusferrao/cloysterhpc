/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cluster.h"
#include "functions.h"
#include "headnode.h"
#include "services/log.h"
#include "services/xcat.h"

#include <iostream>
#include <memory>
#include <regex>

#ifndef NDEBUG
#include <fmt/format.h>
#endif

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

// The rule of zero
// Cluster::Cluster() = default;
// Cluster::~Cluster() = default;

Headnode& Cluster::getHeadnode() { return m_headnode; }

const Headnode& Cluster::getHeadnode() const { return m_headnode; }

std::string_view Cluster::getName() const { return m_name; }

void Cluster::setName(std::string_view name) { m_name = name; }

std::string_view Cluster::getCompanyName() const { return m_companyName; }

void Cluster::setCompanyName(std::string_view companyName)
{
    m_companyName = companyName;
}

std::string_view Cluster::getAdminMail() const { return m_adminMail; }

void Cluster::setAdminMail(std::string_view adminMail)
{
    m_adminMail = adminMail;
}

bool Cluster::isFirewall() const { return m_firewall; }

void Cluster::setFirewall(bool firewall) { m_firewall = firewall; }

Cluster::SELinuxMode Cluster::getSELinux() const { return m_selinux; }

void Cluster::setSELinux(Cluster::SELinuxMode mode) { m_selinux = mode; }

Timezone& Cluster::getTimezone() { return m_timezone; }

void Cluster::setTimezone(const std::string& tz) { m_timezone.setTimezone(tz); }

const std::string& Cluster::getLocale() const { return m_locale; }

void Cluster::setLocale(const std::string& locale) { m_locale = locale; }

const std::string& Cluster::getDomainName() const { return m_domainName; }

/* TODO: Fix logic, split domain to a vector after each dot (.) to check for
 *  correctness
 */
void Cluster::setDomainName(const std::string& domainName)
{
    if (domainName.size() > 255)
        throw;

#if __cpp_lib_starts_ends_with >= 201711L
    if (domainName.starts_with('-') or domainName.ends_with('-'))
#else
    if (boost::algorithm::starts_with(domainName, "-")
        or boost::algorithm::ends_with(domainName, "-"))
#endif
        throw std::runtime_error("Invalid domain name");

    /* Check if string has only digits */
    if (std::regex_match(domainName, std::regex("^[0-9]+$")))
        throw;

    /* Check if it's not only alphanumerics and - */
    if (!(std::regex_match(domainName, std::regex("^[A-Za-z0-9-.]+$"))))
        throw;

    m_domainName = domainName;

    // Force FQDN update if domainName is changed:
    m_headnode.setFQDN(
        fmt::format("{}.{}", m_headnode.getHostname(), m_domainName));
}

std::list<std::unique_ptr<Network>>& Cluster::getNetworks()
{
    return m_network;
}

Network& Cluster::getNetwork(Network::Profile profile)
{
    for (auto& network : m_network) {
        if (network->getProfile() == profile) {
            return *network;
        }
    }

    throw std::runtime_error(
        fmt::format("Cannot get any network with the profile {}",
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

void Cluster::addNetwork()
{
    m_network.emplace_back(std::make_unique<Network>());
}

void Cluster::addNetwork(Network::Profile profile)
{
    m_network.emplace_back(std::make_unique<Network>(profile));
}

void Cluster::addNetwork(Network::Profile profile, Network::Type type)
{
    m_network.emplace_back(std::make_unique<Network>(profile, type));
}

void Cluster::addNetwork(Network::Profile profile, Network::Type type,
    const address& ip, const address& subnetMask, const address& gateway,
    const uint16_t& vlan, const std::string& domainName,
    const std::vector<address>& nameserver)
{

    m_network.emplace_back(std::make_unique<Network>(
        profile, type, ip, subnetMask, gateway, vlan, domainName, nameserver));
}

void Cluster::addNetwork(std::unique_ptr<Network>&& network)
{
    m_network.emplace_back(std::move(network));
}

bool Cluster::isUpdateSystem() const { return m_updateSystem; }

void Cluster::setUpdateSystem(bool updateSystem)
{
    m_updateSystem = updateSystem;
}

Cluster::Provisioner Cluster::getProvisioner() const { return m_provisioner; }

void Cluster::setProvisioner(Cluster::Provisioner provisioner)
{
    m_provisioner = provisioner;
}

std::optional<OFED> Cluster::getOFED() const { return m_ofed; }

void Cluster::setOFED(OFED::Kind kind) { m_ofed = OFED(kind); }

std::optional<std::unique_ptr<QueueSystem>>& Cluster::getQueueSystem()
{
    return m_queueSystem;
}

void Cluster::setQueueSystem(QueueSystem::Kind kind)
{
    switch (kind) {
        case QueueSystem::Kind::None:
            m_queueSystem = std::nullopt;
            break;

        case QueueSystem::Kind::SLURM:
            // m_queueSystem = std::unique_ptr<QueueSystem>(new SLURM());
            m_queueSystem = std::make_unique<SLURM>(*this);
            break;

        case QueueSystem::Kind::PBS:
            // m_queueSystem = std::unique_ptr<QueueSystem>(new PBS());
            m_queueSystem = std::make_unique<PBS>(*this);
            break;
    }
}

std::optional<Postfix>& Cluster::getMailSystem() { return m_mailSystem; }

void Cluster::setMailSystem(Postfix::Profile profile)
{
    m_mailSystem = Postfix(profile);
}

const std::filesystem::path& Cluster::getDiskImage() const
{
    return m_diskImage.getPath();
}

void Cluster::setDiskImage(const std::filesystem::path& diskImagePath)
{
    m_diskImage.setPath(diskImagePath);
}

const std::vector<Node>& Cluster::getNodes() const { return m_nodes; }

void Cluster::addNode(std::string_view hostname, OS& os, CPU& cpu,
    std::list<Connection>&& connections)
{

    m_nodes.emplace_back(hostname, os, cpu, std::move(connections));
}

void Cluster::addNode(std::string_view hostname, OS& os, CPU& cpu,
    std::list<Connection>&& connections, BMC& bmc)
{

    m_nodes.emplace_back(hostname, os, cpu, std::move(connections), bmc);
}

#ifndef NDEBUG
void Cluster::printNetworks(
    const std::list<std::unique_ptr<Network>>& networks) const
{

    LOG_DEBUG("Dump network data:");

#if __cplusplus < 202002L
    size_t i, j;
    for (const auto& network : networkType) {
        i = 0;
#else
    for (size_t i = 0; const auto& network : networks) {
#endif
        LOG_DEBUG("Network [{}]", i++);
        LOG_DEBUG("Profile: {}", magic_enum::enum_name(network->getProfile()));
        LOG_DEBUG("Address: {}", network->getAddress().to_string());
        LOG_DEBUG("Subnet Mask: {}", network->getSubnetMask().to_string());
        LOG_DEBUG("Gateway: {}", network->getGateway().to_string());
        LOG_DEBUG("VLAN: {}", network->getVLAN());
        LOG_DEBUG("Domain Name: {}", network->getDomainName());
#if __cplusplus < 202002L
        j = 0;
        for (const auto& nameserver : network.getNameserver()) {
#else
        for (size_t j = 0; const auto& nameserver : network->getNameservers()) {
#endif
            LOG_DEBUG("Nameserver [{}]: {}", j++, nameserver.to_string());
        }
    }
}

void Cluster::printConnections()
{
    for (std::size_t i { 0 };
         const auto& connection : getHeadnode().getConnections())
        connection.dumpConnection();
}

void Cluster::printData()
{
    LOG_DEBUG("Dump cluster data:");
    LOG_DEBUG("Cluster attributes defined:");
    LOG_DEBUG("OS Data:")
    m_headnode.getOS().printData();
    LOG_DEBUG("Timezone: {}", getTimezone().getTimezone());
    LOG_DEBUG("Locale: {}", getLocale());
    LOG_DEBUG("Hostname: {}", this->m_headnode.getHostname());
    LOG_DEBUG("DomainName: {}", getDomainName());
    LOG_DEBUG("FQDN: {}", this->m_headnode.getFQDN());

    printNetworks(m_network);
    printConnections();

    LOG_DEBUG("Provisioner: {}", static_cast<int>(getProvisioner()));
    //    LOG_DEBUG("nodePrefix: {}", nodePrefix);
    //    LOG_DEBUG("nodePadding: {}", nodePadding);
    //    LOG_DEBUG("nodeStartIP: {}", nodeStartIP);
    //    LOG_DEBUG("nodeRootPassword: {}", nodeRootPassword);
    LOG_DEBUG("nodeDiskImage: {}", getDiskImage().string());

    LOG_DEBUG("Update system: {}", (isUpdateSystem() ? "true" : "false"));
    //    LOG_DEBUG("Remote access: {}", (remoteAccess ? "true" : "false"));

    LOG_DEBUG("Firewall: {}", (isFirewall() ? "true" : "false"));
    LOG_DEBUG("SELinux: {}", static_cast<int>(getSELinux()));
}

void Cluster::fillTestData()
{
    setName("Cloyster");
    setFirewall(true);
    setSELinux(SELinuxMode::Disabled);
    setTimezone("America/Sao_Paulo");
    setLocale("en_US.UTF-8");
    this->m_headnode.setHostname(std::string_view { "headnode" });
    setDomainName("cluster.example.tld");
    this->m_headnode.setFQDN(fmt::format(
        "{0}.{1}", this->m_headnode.getHostname(), getDomainName()));

    setOFED(OFED::Kind::Inbox);
    setQueueSystem(QueueSystem::Kind::SLURM);
    m_queueSystem.value()->setDefaultQueue("Execution");

    addNetwork(Network::Profile::External, Network::Type::Ethernet,
        boost::asio::ip::make_address("172.16.144.0"),
        boost::asio::ip::make_address("255.255.255.0"),
        boost::asio::ip::make_address("172.16.144.1"), 0, "home.ferrao.net.br",
        { boost::asio::ip::make_address("172.16.144.1") });
    addNetwork(Network::Profile::Management, Network::Type::Ethernet,
        boost::asio::ip::make_address("172.26.0.0"),
        boost::asio::ip::make_address("255.255.0.0"),
        boost::asio::ip::make_address("0.0.0.0"), 0, "cluster.example.tld",
        { boost::asio::ip::make_address("172.26.0.1") });
    addNetwork(Network::Profile::Application, Network::Type::Infiniband,
        boost::asio::ip::make_address("172.27.0.0"),
        boost::asio::ip::make_address("255.255.0.0"),
        boost::asio::ip::make_address("0.0.0.0"), 0, "ib.cluster.example.tld",
        { boost::asio::ip::make_address("172.27.255.254") });
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

    m_headnode.addConnection(getNetwork(Network::Profile::External), "enp0s25",
        "de:ad:be:ff:00:00", boost::asio::ip::make_address("172.16.144.50"));
    m_headnode.addConnection(getNetwork(Network::Profile::Management), "eno1",
        "de:ad:be:ff:00:01", boost::asio::ip::make_address("172.26.255.254"));
    // It's ethernet, we know, but consider as Infiniband
    m_headnode.addConnection(getNetwork(Network::Profile::Application), "eno1",
        "de:ad:be:ff:00:02", boost::asio::ip::make_address("172.27.255.254"));

#if 0
    m_headnode.addConnection(getNetwork(Network::Profile::Service),
                             "ens224", "192.168.22.8");
#endif

    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);

    setDiskImage("/root/OracleLinux-R8-U5-x86_64-dvd.iso");
    OS nodeOS(OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el8,
        OS::Distro::OL, "5.4.17-2136.302.6.1.el8uek.x86_64", 8, 5);
    CPU nodeCPU(2, 4, 2);

    // TODO: Pass network connection as object
    std::list<Connection> connections1 {
        { &getNetwork(Network::Profile::Management), {}, "00:0c:29:9b:0c:75",
            boost::asio::ip::make_address("172.26.0.1") },
        { &getNetwork(Network::Profile::Application), "eno1", {},
            boost::asio::ip::make_address("172.27.0.1") }
    };

    std::list<Connection> connections2 {
        { &getNetwork(Network::Profile::Management), {}, "de:ad:be:ff:00:00",
            boost::asio::ip::make_address("172.26.0.2") }
    };

    BMC bmc { "172.25.0.2", "ADMIN", "ADMIN" };

    addNode("n01", nodeOS, nodeCPU, std::move(connections1));
    addNode("n02", nodeOS, nodeCPU, std::move(connections2), bmc);

    /* Bad and old data */
    nodePrefix = "n";
    nodePadding = 2;
    nodeStartIP = boost::asio::ip::make_address("172.26.0.1");
    nodeRootPassword = "pwdNodeRoot";
}
#endif
