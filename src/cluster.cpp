/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/answerfile.h>
#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/cluster.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/headnode.h>
#include <cloysterhpc/inifile.h>
#include <cloysterhpc/runner.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/xcat.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <expected>
#include <iostream>
#include <memory>
#include <regex>

#ifndef NDEBUG
#include <fmt/format.h>
#endif

#if __cpp_lib_starts_ends_with < 201711L
#include <boost/algorithm/string.hpp>
#endif

Cluster::Cluster()
{
    if (cloyster::dryRun) {
        m_runner = std::make_unique<DryRunner>();
    } else {
        m_runner = std::make_unique<Runner>();
    }

    m_systemdBus = std::make_shared<DBusClient>(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1");
}
// The rule of zero
// Cluster::~Cluster() = default;

std::shared_ptr<DBusClient> Cluster::getDaemonBus() { return m_systemdBus; }

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

const Locale& Cluster::getLocale() const { return m_locale; }

void Cluster::setLocale(const Locale& locale) { m_locale = locale; }

void Cluster::setLocale(const std::string& locale)
{
    m_locale.setLocale(locale);
}

const std::string Cluster::getDomainName() const
{
    std::string fqdn = m_headnode.getFQDN();
    return fqdn.substr(fqdn.find_first_of('.') + 1);
}

void Cluster::setDomainName(const std::string& domainName)
{
    // Force FQDN update if domainName is changed:
    m_headnode.setFQDN(
        fmt::format("{}.{}", m_headnode.getHostname(), domainName));
}

std::list<std::unique_ptr<Network>>& Cluster::getNetworks()
{
    return m_network;
}

void Cluster::initRepoManager()
{
    m_repos.emplace(*m_runner, m_headnode.getOS());
}

RepoManager& Cluster::getRepoManager()
{
    if (!m_repos) {
        initRepoManager();
    }

    return m_repos.value();
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
    const std::string& ip, const std::string& subnetMask,
    const std::string& gateway, const uint16_t& vlan,
    const std::string& domainName, const std::vector<address>& nameserver)
{
    m_network.emplace_back(std::make_unique<Network>(
        profile, type, ip, subnetMask, gateway, vlan, domainName, nameserver));
}

void Cluster::addNetwork(Network::Profile profile, Network::Type type,
    const std::string& ip, const std::string& subnetMask,
    const std::string& gateway, const uint16_t& vlan,
    const std::string& domainName, const std::vector<std::string>& nameserver)
{
    std::vector<address> formattedNameservers;
    for (std::size_t i = 0; i < nameserver.size(); i++) {
        formattedNameservers.emplace_back(
            boost::asio::ip::make_address(nameserver[i]));
    }

    addNetwork(profile, type, ip, subnetMask, gateway, vlan, domainName,
        formattedNameservers);
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
    m_mailSystem.emplace(m_systemdBus, *m_runner, profile);
}

const std::filesystem::path& Cluster::getDiskImage() const
{
    return m_diskImage.getPath();
}

void Cluster::setDiskImage(const std::filesystem::path& diskImagePath)
{
    // BUG: This does not hanble ~ expansion for userdir
    if (std::filesystem::exists(diskImagePath)) {
        m_diskImage.setPath(diskImagePath);
    } else {
        throw std::runtime_error(fmt::format(
            "Disk image path {} doesn't exist", diskImagePath.string()));
    }
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

void Cluster::addNode(Node node) { m_nodes.emplace_back(node); }

#ifndef NDEBUG
void Cluster::printNetworks(
    const std::list<std::unique_ptr<Network>>& networks) const
{

    LOG_DEBUG("Dump network data:")

#if __cplusplus < 202002L
    size_t i, j;
    for (const auto& network : networkType) {
        i = 0;
#else
    for (size_t i = 0; const auto& network : networks) {
#endif
        LOG_DEBUG("Network [{}]", i++)
        LOG_DEBUG("Profile: {}", magic_enum::enum_name(network->getProfile()))
        LOG_DEBUG("Address: {}", network->getAddress().to_string())
        LOG_DEBUG("Subnet Mask: {}", network->getSubnetMask().to_string())
        LOG_DEBUG("Gateway: {}", network->getGateway().to_string())
        LOG_DEBUG("VLAN: {}", network->getVLAN())
        LOG_DEBUG("Domain Name: {}", network->getDomainName())
#if __cplusplus < 202002L
        j = 0;
        for (const auto& nameserver : network.getNameserver()) {
#else
        for (size_t j = 0; const auto& nameserver : network->getNameservers()) {
#endif
            LOG_DEBUG("Nameserver [{}]: {}", j++, nameserver.to_string())
        }
    }
}

void Cluster::printConnections()
{
    for (const auto& connection : getHeadnode().getConnections()) {
        connection.dumpConnection();
    }
}

void Cluster::printData()
{
    LOG_DEBUG("Dump cluster data:")
    LOG_DEBUG("Cluster attributes defined:")
    LOG_DEBUG("OS Data:")
    m_headnode.getOS().printData();
    LOG_DEBUG("Timezone: {}", getTimezone().getTimezone())
    LOG_DEBUG("Locale: {}", getLocale().getLocale())
    LOG_DEBUG("Hostname: {}", this->m_headnode.getHostname())
    LOG_DEBUG("DomainName: {}", getDomainName())
    LOG_DEBUG("FQDN: {}", this->m_headnode.getFQDN())

    printNetworks(m_network);
    printConnections();

    LOG_DEBUG("Provisioner: {}", static_cast<int>(getProvisioner()))
    //    LOG_DEBUG("nodePrefix: {}", nodePrefix)
    //    LOG_DEBUG("nodePadding: {}", nodePadding)
    //    LOG_DEBUG("nodeStartIP: {}", nodeStartIP)
    //    LOG_DEBUG("nodeRootPassword: {}", nodeRootPassword)
    LOG_DEBUG("nodeDiskImage: {}", getDiskImage().string())

    LOG_DEBUG("Update system: {}", (isUpdateSystem() ? "true" : "false"))
    //    LOG_DEBUG("Remote access: {}", (remoteAccess ? "true" : "false"))

    LOG_DEBUG("Firewall: {}", (isFirewall() ? "true" : "false"))
    LOG_DEBUG("SELinux: {}", static_cast<int>(getSELinux()))
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
    m_queueSystem.value()->setDefaultQueue("execution");

    addNetwork(Network::Profile::External, Network::Type::Ethernet,
        "172.16.144.0", "255.255.255.0", "172.16.144.1", 0,
        "home.ferrao.net.br",
        { boost::asio::ip::make_address("172.16.144.1") });
    addNetwork(Network::Profile::Management, Network::Type::Ethernet,
        "172.26.0.0", "255.255.0.0", "0.0.0.0", 0, "cluster.example.tld",
        { boost::asio::ip::make_address("172.26.0.1") });
    addNetwork(Network::Profile::Application, Network::Type::Infiniband,
        "172.27.0.0", "255.255.0.0", "0.0.0.0", 0, "ib.cluster.example.tld",
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
        "de:ad:be:ff:00:00", "172.16.144.50");
    m_headnode.addConnection(getNetwork(Network::Profile::Management), "eno1",
        "de:ad:be:ff:00:01", "172.26.255.254");
    // It's ethernet, we know, but consider as Infiniband
    m_headnode.addConnection(getNetwork(Network::Profile::Application), "eno1",
        "de:ad:be:ff:00:02", "172.27.255.254");

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
            "172.26.0.1" },
        { &getNetwork(Network::Profile::Application), "eno1", {}, "172.27.0.1" }
    };

    std::list<Connection> connections2 { { &getNetwork(
                                               Network::Profile::Management),
        {}, "de:ad:be:ff:00:00", "172.26.0.2" } };

    BMC bmc { "172.25.0.2", "ADMIN", "ADMIN", 0, 115200, BMC::kind::IPMI };

    addNode("n01", nodeOS, nodeCPU, std::move(connections1));
    addNode("n02", nodeOS, nodeCPU, std::move(connections2), bmc);

    /* Bad and old data */
    nodePrefix = "n";
    nodePadding = 2;
    nodeStartIP = boost::asio::ip::make_address("172.26.0.1");
    nodeRootPassword = std::string { "NOSONARpwdNodeRoot" }.substr(7);
}
#endif

void Cluster::fillData(const std::string& answerfilePath)
{

    AnswerFile answerfile(answerfilePath);

    LOG_TRACE("Configure Management Network")
    // Management Network
    auto managementNetwork = std::make_unique<Network>(
        Network::Profile::Management, Network::Type::Ethernet);

    managementNetwork->setSubnetMask(answerfile.management.subnet_mask.value());

    if (answerfile.management.gateway.has_value()) {
        managementNetwork->setGateway(answerfile.management.gateway.value());
    }

    managementNetwork->setDomainName(answerfile.management.domain_name.value());

    if (answerfile.management.nameservers.has_value()) {
        managementNetwork->setNameservers(
            answerfile.management.nameservers.value());
    } else {
        managementNetwork->setNameservers(
            managementNetwork->fetchNameservers());
    }

    managementNetwork->setAddress(managementNetwork->calculateAddress(
        answerfile.management.con_ip_addr.value()));

    LOG_TRACE("Configure External Network")
    // External Network
    auto externalNetwork = std::make_unique<Network>(
        Network::Profile::External, Network::Type::Ethernet);

    if (answerfile.external.subnet_mask.has_value()) {
        externalNetwork->setSubnetMask(answerfile.external.subnet_mask.value());
    } else {
        externalNetwork->setSubnetMask(externalNetwork->fetchSubnetMask(
            answerfile.external.con_interface.value()));
    }

    if (answerfile.external.gateway.has_value()) {
        externalNetwork->setGateway(answerfile.external.gateway.value());
    }

    if (!answerfile.external.domain_name->empty()) {
        externalNetwork->setDomainName(answerfile.external.domain_name.value());
    } else {
        externalNetwork->setDomainName(externalNetwork->fetchDomainName());
    }

    if (answerfile.external.nameservers.has_value()) {
        externalNetwork->setNameservers(
            answerfile.external.nameservers.value());
    } else {
        externalNetwork->setNameservers(externalNetwork->fetchNameservers());
    }

    // System
    setDiskImage(answerfile.system.disk_image);

    // OS and Information

    LOG_INFO("Distro: {}", magic_enum::enum_name(answerfile.system.distro));
    LOG_INFO("Kernel: {}", answerfile.system.kernel);
    LOG_INFO("Version: {}", answerfile.system.version);

    OS nodeOS;
    nodeOS.setArch(OS::Arch::x86_64);
    nodeOS.setFamily(OS::Family::Linux);
    nodeOS.setDistro(answerfile.system.distro);
    nodeOS.setKernel(answerfile.system.kernel);
    nodeOS.setVersion(answerfile.system.version);

    LOG_TRACE("Cluster name: {}", answerfile.information.cluster_name)

    setName(answerfile.information.cluster_name);
    setCompanyName(answerfile.information.company_name);
    setAdminMail(answerfile.information.administrator_email);

    setTimezone(answerfile.time.timezone);
    setLocale(answerfile.time.locale);

    this->m_headnode.setHostname(answerfile.hostname.hostname);
    setDomainName(answerfile.hostname.domain_name);
    this->m_headnode.setFQDN(fmt::format(
        "{0}.{1}", this->m_headnode.getHostname(), getDomainName()));

    setOFED(OFED::Kind::Inbox);
    setQueueSystem(QueueSystem::Kind::SLURM);
    m_queueSystem.value()->setDefaultQueue("execution");

    addNetwork(std::move(managementNetwork));

    LOG_TRACE("Configure Management Connection")
    auto managementConnection
        = Connection(&getNetwork(Network::Profile::Management));
    managementConnection.setInterface(
        answerfile.management.con_interface.value());

    managementConnection.setAddress(answerfile.management.con_ip_addr.value());

    if (!answerfile.management.con_mac_addr->empty()) {
        managementConnection.setMAC(answerfile.management.con_mac_addr.value());
    }

    getHeadnode().addConnection(std::move(managementConnection));

    addNetwork(std::move(externalNetwork));

    LOG_TRACE("Configure External Connection")
    auto externalConnection
        = Connection(&getNetwork(Network::Profile::External));
    externalConnection.setInterface(answerfile.external.con_interface.value());

    if (answerfile.external.con_ip_addr.has_value()) {
        externalConnection.setAddress(answerfile.external.con_ip_addr.value());

        getNetwork(Network::Profile::External)
            .setAddress(
                getNetwork(Network::Profile::External)
                    .calculateAddress(answerfile.external.con_ip_addr.value()));
    } else {
        auto externalNetworkIpAddress = externalConnection.fetchAddress(
            answerfile.external.con_interface.value());
        externalConnection.setAddress(externalNetworkIpAddress);

        getNetwork(Network::Profile::External)
            .setAddress(getNetwork(Network::Profile::External)
                            .calculateAddress(externalNetworkIpAddress));
    }

    if (!answerfile.external.con_mac_addr->empty()) {
        externalConnection.setMAC(answerfile.external.con_mac_addr.value());
    }

    getHeadnode().addConnection(std::move(externalConnection));

    // Service Network
    if (answerfile.service.con_interface.has_value()) {
        LOG_TRACE("Configure Service Network")
        auto serviceNetwork = std::make_unique<Network>(
            Network::Profile::Service, Network::Type::Ethernet);

        if (answerfile.service.subnet_mask.has_value()) {
            serviceNetwork->setSubnetMask(
                answerfile.service.subnet_mask.value());
        } else {
            serviceNetwork->setSubnetMask(externalNetwork->fetchSubnetMask(
                answerfile.service.con_interface.value()));
        }

        if (answerfile.service.gateway.has_value()) {
            serviceNetwork->setGateway(answerfile.service.gateway.value());
        } else {
            serviceNetwork->setGateway(serviceNetwork->fetchGateway(
                answerfile.service.con_interface.value()));
        }

        if (!answerfile.service.domain_name->empty()) {
            serviceNetwork->setDomainName(
                answerfile.service.domain_name.value());
        } else {
            serviceNetwork->setDomainName(serviceNetwork->fetchDomainName());
        }

        if (answerfile.service.nameservers.has_value()) {
            serviceNetwork->setNameservers(
                answerfile.service.nameservers.value());
        } else {
            serviceNetwork->setNameservers(serviceNetwork->fetchNameservers());
        }

        addNetwork(std::move(serviceNetwork));

        auto serviceConnection
            = Connection(&getNetwork(Network::Profile::Service));

        if (!answerfile.service.con_mac_addr->empty()) {
            serviceConnection.setMAC(answerfile.service.con_mac_addr.value());
        }

        serviceConnection.setInterface(
            answerfile.application.con_interface.value());
        serviceConnection.setAddress(answerfile.service.con_ip_addr.value());

        getNetwork(Network::Profile::Service)
            .setAddress(
                getNetwork(Network::Profile::Service)
                    .calculateAddress(answerfile.service.con_ip_addr.value()));

        m_headnode.addConnection(std::move(serviceConnection));
    }

    // Infiniband (Application) Network
    if (answerfile.application.con_interface.has_value()) {
        LOG_TRACE("Configure Application Network")
        auto applicationNetwork = std::make_unique<Network>(
            Network::Profile::Application, Network::Type::Ethernet);

        auto& subnet_mask = answerfile.application.subnet_mask;
        auto& gateway = answerfile.application.gateway;
        auto& domain_name = answerfile.application.domain_name;
        auto& nameservers = answerfile.application.nameservers;

        auto throwIfEmpty = [](bool optional_cast_value,
                                const char* fieldname) {
            if (!optional_cast_value) {
                throw answerfile_validation_exception(fmt::format(
                    "Field {} of application network is empty", fieldname));
            }
        };
#define THROW_IF_EMPTY(field) throwIfEmpty(field.has_value(), #field)
        THROW_IF_EMPTY(subnet_mask);
        THROW_IF_EMPTY(gateway);
        THROW_IF_EMPTY(domain_name);
        THROW_IF_EMPTY(nameservers);
#undef THROW_IF_EMPTY

        applicationNetwork->setSubnetMask(subnet_mask.value());
        applicationNetwork->setGateway(gateway.value());
        applicationNetwork->setDomainName(domain_name.value());
        applicationNetwork->setNameservers(nameservers.value());

        addNetwork(std::move(applicationNetwork));

        auto applicationConnection
            = Connection(&getNetwork(Network::Profile::Application));

        if (!answerfile.application.con_mac_addr->empty()) {
            applicationConnection.setMAC(
                answerfile.application.con_mac_addr.value());
        }

        applicationConnection.setInterface(
            answerfile.application.con_interface.value());

        applicationConnection.setAddress(
            answerfile.application.con_ip_addr.value());

        getNetwork(Network::Profile::Application)
            .setAddress(getNetwork(Network::Profile::Application)
                            .calculateAddress(
                                answerfile.application.con_ip_addr.value()));

        m_headnode.addConnection(std::move(applicationConnection));
    }

    // System
    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);
    // BUG: Headnode OS may not be the same as the node OS
    // m_headnode.setOS(nodeOS);

    for (const auto& tool : answerfile.getTools()) {
        tool->install();
    }

    LOG_INFO("Configure Nodes")
    for (const auto& node : answerfile.nodes.nodes) {
        LOG_TRACE("Configure node {}", node.hostname.value())

        std::list<Connection> nodeConnections;
        auto& connection = nodeConnections.emplace_back(
            &getNetwork(Network::Profile::Management));

        Node newNode;
        CPU newNodeCPU;
        BMC newNodeBMC;

        std::string nodename = node.hostname.value();
        newNode.setHostname(nodename);

        newNode.setNodeStartIp(node.start_ip.value());

        LOG_TRACE("{} start ip: {}", newNode.getHostname(),
            newNode.getNodeStartIp()->to_string());

        auto& mac_address = node.mac_address;
        if (mac_address) {
            if (auto err = Connection::validateMAC(mac_address.value());
                !err.has_value()) {
                throw answerfile_validation_exception { fmt::format(
                    "Error decoding MAC address (read {}) of node {}: {}",
                    mac_address.value(), nodename, err.error()) };
            } else {
                newNode.setMACAddress(mac_address.value());
            }
        } else {
            throw answerfile_validation_exception { fmt::format(
                "Missing MAC address on node {}", nodename) };
        }
        LOG_TRACE("{} MAC address: {}", newNode.getHostname(),
            newNode.getMACAddress());

        newNode.setNodeRootPassword(node.root_password.value());

        LOG_TRACE("{} root password: {}", newNode.getHostname(),
            newNode.getNodeRootPassword().value());

        auto convertOrError
            = [nodename](std::string value, const char* field_name) {
                  try {
                      return std::stoul(value);
                  } catch (std::invalid_argument& e) {
                      throw answerfile_validation_exception { fmt::format(
                          "Conversion error on node {}: field {} is not a "
                          "number (value is {})",
                          nodename, field_name, value) };
                  }
              };

#define CONVERT_OR_ERROR(FIELD) convertOrError(node.FIELD.value(), #FIELD)

        newNodeCPU.setSockets(CONVERT_OR_ERROR(sockets));

        LOG_TRACE("{} CPU sockets: {}", newNode.getHostname(),
            newNodeCPU.getSockets());

        newNodeCPU.setCoresPerSocket(CONVERT_OR_ERROR(cores_per_socket));

        LOG_TRACE("{} CPU cores per socket: {}", newNode.getHostname(),
            newNodeCPU.getCoresPerSocket());

        newNodeCPU.setThreadsPerCore(CONVERT_OR_ERROR(threads_per_core));

        LOG_TRACE("{} CPU threads per core: {}", newNode.getHostname(),
            newNodeCPU.getThreadsPerCore());

        newNodeBMC.setAddress(node.bmc_address.value());

        LOG_TRACE("{} BMC address: {}", newNode.getHostname(),
            newNodeBMC.getAddress());

        newNodeBMC.setUsername(node.bmc_username.value());

        LOG_TRACE("{} BMC username: {}", newNode.getHostname(),
            newNodeBMC.getUsername());

        newNodeBMC.setPassword(node.bmc_password.value());

        LOG_TRACE("{} BMC password: {}", newNode.getHostname(),
            newNodeBMC.getPassword());

        newNodeBMC.setSerialPort(CONVERT_OR_ERROR(bmc_serialport));

        LOG_TRACE("{} BMC serial port: {}", newNode.getHostname(),
            newNodeBMC.getSerialPort());

        newNodeBMC.setSerialSpeed(CONVERT_OR_ERROR(bmc_serialspeed));

        LOG_TRACE("{} BMC serial speed: {}", newNode.getHostname(),
            newNodeBMC.getSerialSpeed());

#undef CONVERT_OR_ERROR

        newNode.setCPU(newNodeCPU);
        newNode.setBMC(newNodeBMC);
        newNode.setOS(nodeOS);

        connection.setMAC(newNode.getMACAddress());
        connection.setAddress(newNode.getNodeStartIp().value());
        newNode.setConnection(nodeConnections);

        addNode(newNode);
    }

    if (answerfile.postfix.enabled) {
        setMailSystem(answerfile.postfix.profile);
        m_mailSystem->setHostname(this->m_headnode.getHostname());
        m_mailSystem->setDomain(getDomainName());
        m_mailSystem->setDestination(answerfile.postfix.destination);

        if (!m_mailSystem->getDomain()) {
            throw answerfile_validation_exception(
                "A domain is needed for e-mail configuration");
        }

        switch (answerfile.postfix.profile) {
            case Postfix::Profile::Local:
                break;
            case Postfix::Profile::Relay:
                m_mailSystem->setSMTPServer(
                    answerfile.postfix.smtp.value().server);
                m_mailSystem->setPort(answerfile.postfix.smtp.value().port);
                break;
            case Postfix::Profile::SASL:
                m_mailSystem->setSMTPServer(
                    answerfile.postfix.smtp.value().server);
                m_mailSystem->setPort(answerfile.postfix.smtp.value().port);
                m_mailSystem->setUsername(
                    answerfile.postfix.smtp.value().sasl.value().username);
                m_mailSystem->setPassword(
                    answerfile.postfix.smtp.value().sasl.value().password);
                break;
        }
        m_mailSystem->setCertFile(answerfile.postfix.cert_file);
        m_mailSystem->setKeyFile(answerfile.postfix.key_file);
    }

    /* Bad and old data - @TODO Must improve */
    nodeStartIP = answerfile.nodes.generic->start_ip.value();
    nodeRootPassword = answerfile.nodes.generic->root_password.value();
}
