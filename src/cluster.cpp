/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/cluster.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/headnode.h>
#include <cloysterhpc/inifile.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/xcat.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
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
    m_mailSystem = Postfix(profile);
}

const std::filesystem::path& Cluster::getDiskImage() const
{
    return m_diskImage.getPath();
}

void Cluster::setDiskImage(const std::filesystem::path& diskImagePath)
{
    if (std::filesystem::exists(diskImagePath)) {
        m_diskImage.setPath(diskImagePath);
    } else {
        throw std::runtime_error("Disk image path doesn't exist");
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
    for (const auto& connection : getHeadnode().getConnections()) {
        connection.dumpConnection();
    }
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
    nodeRootPassword = "pwdNodeRoot";
}
#endif

void Cluster::fillData(const std::string& answerfilePath)
{
    // @TODO Clean and improve this code

    auto lGenericWarnMustFillSectionKey
        = [](std::string section, std::string key) {
              return fmt::format(
                  "Answerfile section \"{}\" must have \"{}\" key filled",
                  section, key);
          };

    inifile ini;

    ini.loadFile(answerfilePath);

    LOG_TRACE("Read answerfile variables:");

    // Information
    auto clusterName = ini.getValue("information", "cluster_name");
    auto companyName = ini.getValue("information", "company_name");
    auto administratorEmail
        = ini.getValue("information", "administrator_email");

    // Time
    auto timezone = ini.getValue("time", "timezone");
    auto timeserver = ini.getValue("time", "timeserver");
    auto locale = ini.getValue("time", "locale");

    // Hostname
    auto hostname = ini.getValue("hostname", "hostname");
    auto domainName = ini.getValue("hostname", "domain_name");

    // Management Network
    auto managementNetwork = std::make_unique<Network>(
        Network::Profile::Management, Network::Type::Ethernet);

    auto managementNetworkInterface
        = ini.getValue("network_management", "interface");

    auto managementNetworkSubnetMask
        = ini.getValue("network_management", "subnet_mask");
    managementNetwork->setSubnetMask(managementNetworkSubnetMask);

    if (ini.exists("network_management", "gateway")) {
        auto managementNetworkGateway
            = ini.getValue("network_management", "gateway");
        managementNetwork->setGateway(managementNetworkGateway);
    }

    auto managementNetworkDomainName
        = ini.getValue("network_management", "domain_name");
    managementNetwork->setDomainName(managementNetworkDomainName);

    if (ini.exists("network_management", "nameservers")) {
        std::vector<std::string> managementNetworkNameservers;
        boost::split(managementNetworkNameservers,
            ini.getValue("network_management", "nameservers"),
            boost::is_any_of(", "), boost::token_compress_on);

        managementNetwork->setNameservers(managementNetworkNameservers);
    } else {
        managementNetwork->setNameservers(
            managementNetwork->fetchNameservers());
    }

    auto managementConnectionIpAddress
        = ini.getValue("network_management", "ip_address");

    managementNetwork->setAddress(
        managementNetwork->calculateAddress(managementConnectionIpAddress));

    // External Network
    auto externalNetwork = std::make_unique<Network>(
        Network::Profile::External, Network::Type::Ethernet);

    auto externalNetworkInterface
        = ini.getValue("network_external", "interface");

    if (ini.exists("network_external", "subnet_mask")) {
        auto externalNetworkSubnetMask
            = ini.getValue("network_external", "subnet_mask");
        externalNetwork->setSubnetMask(externalNetworkSubnetMask);
    } else {
        externalNetwork->setSubnetMask(
            externalNetwork->fetchSubnetMask(externalNetworkInterface));
    }

    if (ini.exists("network_external", "gateway")) {
        auto externalNetworkGateway
            = ini.getValue("network_external", "gateway");
        externalNetwork->setGateway(externalNetworkGateway);
    }

    if (ini.exists("network_external", "domain_name")) {
        auto externalNetworkDomainName
            = ini.getValue("network_external", "domain_name");
        externalNetwork->setDomainName(externalNetworkDomainName);
    } else {
        externalNetwork->setDomainName(externalNetwork->fetchDomainName());
    }

    if (ini.exists("network_external", "nameservers")) {
        std::vector<std::string> externalNetworkNameservers;
        boost::split(externalNetworkNameservers,
            ini.getValue("network_external", "nameservers"),
            boost::is_any_of(", "), boost::token_compress_on);

        externalNetwork->setNameservers(externalNetworkNameservers);
    } else {
        externalNetwork->setNameservers(externalNetwork->fetchNameservers());
    }

    // System
    std::filesystem::path diskImage = ini.getValue("system", "disk_image");
    setDiskImage(diskImage);

    auto distro = ini.getValue("system", "distro");
    auto distro_version = ini.getValue("system", "version");
    auto kernel = ini.getValue("system", "kernel");

    // Nodes

    OS nodeOS;
    nodeOS.setArch(OS::Arch::x86_64);
    nodeOS.setFamily(OS::Family::Linux);
    nodeOS.setPlatform(OS::Platform::el8);
    nodeOS.setDistro(distro);
    nodeOS.setKernel(kernel);
    nodeOS.setVersion(distro_version);

    Node genericNode;

    if (ini.exists("node")) {

        CPU genericNodeCPU;
        BMC genericNodeBMC;

        if (ini.exists("node", "prefix")) {
            genericNode.setPrefix(ini.getValue("node", "prefix"));
        }

        if (ini.exists("node", "padding")) {
            genericNode.setPadding(std::stoul(ini.getValue("node", "padding")));
        }

        if (ini.exists("node", "node_start_ip")) {
            genericNode.setNodeStartIp(boost::asio::ip::make_address(
                ini.getValue("node", "node_start_ip")));
        }

        if (ini.exists("node", "node_root_password")) {
            genericNode.setNodeRootPassword(
                ini.getValue("node", "node_root_password"));
        }

        if (ini.exists("node", "sockets")) {
            genericNodeCPU.setSockets(
                std::stoul(ini.getValue("node", "sockets")));
        }

        if (ini.exists("node", "cores_per_socket")) {
            genericNodeCPU.setCoresPerSocket(
                std::stoul(ini.getValue("node", "cores_per_socket")));
        }

        if (ini.exists("node", "threads_per_core")) {
            genericNodeCPU.setThreadsPerCore(
                std::stoul(ini.getValue("node", "threads_per_core")));
        }

        if (ini.exists("node", "bmc_address")) {
            genericNodeBMC.setAddress(ini.getValue("node", "bmc_address"));
        }

        if (ini.exists("node", "bmc_username")) {
            genericNodeBMC.setUsername(ini.getValue("node", "bmc_username"));
        }

        if (ini.exists("node", "bmc_password")) {
            genericNodeBMC.setPassword(ini.getValue("node", "bmc_password"));
        }

        if (ini.exists("node", "bmc_serialport")) {
            genericNodeBMC.setSerialPort(
                std::stoul(ini.getValue("node", "bmc_serialport")));
        }

        if (ini.exists("node", "bmc_serialspeed")) {
            genericNodeBMC.setSerialSpeed(
                std::stoul(ini.getValue("node", "bmc_serialspeed")));
        }

        genericNode.setCPU(genericNodeCPU);
        genericNode.setBMC(genericNodeBMC);
    }

    LOG_TRACE("Cluster name: {}", clusterName);

    setName(clusterName);
    setCompanyName(companyName);
    setAdminMail(administratorEmail);

    setTimezone(timezone);
    setLocale(locale);

    this->m_headnode.setHostname(hostname);
    setDomainName(domainName);
    this->m_headnode.setFQDN(fmt::format(
        "{0}.{1}", this->m_headnode.getHostname(), getDomainName()));

    setOFED(OFED::Kind::Inbox);
    setQueueSystem(QueueSystem::Kind::SLURM);
    m_queueSystem.value()->setDefaultQueue("execution");

    addNetwork(std::move(managementNetwork));

    auto managementConnection
        = Connection(&getNetwork(Network::Profile::Management));
    managementConnection.setInterface(managementNetworkInterface);

    managementConnection.setAddress(managementConnectionIpAddress);

    if (ini.exists("network_management", "mac_address")) {
        auto managementNetworkMacAddress
            = ini.getValue("network_management", "mac_address");
        managementConnection.setMAC(managementNetworkMacAddress);
    }

    getHeadnode().addConnection(std::move(managementConnection));

    addNetwork(std::move(externalNetwork));

    auto externalConnection
        = Connection(&getNetwork(Network::Profile::External));
    externalConnection.setInterface(externalNetworkInterface);

    if (ini.exists("network_external", "ip_address")) {
        auto externalNetworkIpAddress
            = ini.getValue("network_external", "ip_address");
        externalConnection.setAddress(externalNetworkIpAddress);

        getNetwork(Network::Profile::External)
            .setAddress(getNetwork(Network::Profile::External)
                            .calculateAddress(externalNetworkIpAddress));

    } else {
        auto externalNetworkIpAddress
            = externalConnection.fetchAddress(externalNetworkInterface);
        externalConnection.setAddress(externalNetworkIpAddress);

        getNetwork(Network::Profile::External)
            .setAddress(getNetwork(Network::Profile::External)
                            .calculateAddress(externalNetworkIpAddress));
    }

    if (ini.exists("network_external", "mac_address")) {
        auto externalNetworkMacAddress
            = ini.getValue("network_external", "mac_address");
        externalConnection.setMAC(externalNetworkMacAddress);
    }

    getHeadnode().addConnection(std::move(externalConnection));

    // Infiniband (Application) Network
    if (ini.exists("network_application")) {
        auto applicationNetwork = std::make_unique<Network>(
            Network::Profile::Application, Network::Type::Ethernet);

        if (ini.exists("network_application", "subnet_mask")) {
            auto applicationNetworkSubnetMask
                = ini.getValue("network_application", "subnet_mask");
            applicationNetwork->setSubnetMask(applicationNetworkSubnetMask);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "subnet_mask"));
        }

        if (ini.exists("network_application", "gateway")) {
            auto applicationNetworkGateway
                = ini.getValue("network_application", "gateway");
            applicationNetwork->setGateway(applicationNetworkGateway);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "gateway"));
        }

        if (ini.exists("network_application", "domain_name")) {
            auto applicationNetworkDomainName
                = ini.getValue("network_application", "domain_name");
            applicationNetwork->setDomainName(applicationNetworkDomainName);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "domain_name"));
        }

        if (ini.exists("network_application", "nameservers")) {
            std::vector<std::string> applicationNetworkNameservers;
            boost::split(applicationNetworkNameservers,
                ini.getValue("network_application", "nameservers"),
                boost::is_any_of(", "), boost::token_compress_on);

            applicationNetwork->setNameservers(applicationNetworkNameservers);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "nameservers"));
        }

        addNetwork(std::move(applicationNetwork));

        auto applicationConnection
            = Connection(&getNetwork(Network::Profile::Application));

        if (ini.exists("network_application", "mac_address")) {
            auto applicationConnectionMacAddress
                = ini.getValue("network_application", "mac_address");
            applicationConnection.setMAC(applicationConnectionMacAddress);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "mac_address"));
        }

        if (ini.exists("network_application", "interface")) {
            auto applicationConnectionInterface
                = ini.getValue("network_application", "interface");
            applicationConnection.setInterface(applicationConnectionInterface);
        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "interface"));
        }

        if (ini.exists("network_application", "ip_address")) {
            auto applicationConnectionIpAddress
                = ini.getValue("network_application", "ip_address");
            applicationConnection.setAddress(applicationConnectionIpAddress);

            getNetwork(Network::Profile::Application)
                .setAddress(
                    getNetwork(Network::Profile::Application)
                        .calculateAddress(applicationConnectionIpAddress));

        } else {
            throw std::runtime_error(lGenericWarnMustFillSectionKey(
                "network_application", "ip_address"));
        }

        m_headnode.addConnection(std::move(applicationConnection));
    }

    // System
    setUpdateSystem(true);
    setProvisioner(Provisioner::xCAT);
    m_headnode.setOS(nodeOS);

    LOG_TRACE("Read nodes data from answerfile");
    int nodeCounter = 1;
    while (true) {

        std::string nodeSection = fmt::format("node.{}", nodeCounter);
        if (!ini.exists(nodeSection)) {
            break;
        }

        LOG_TRACE("Configure {}", nodeSection);

        std::list<Connection> nodeConnections;
        auto& connection = nodeConnections.emplace_back(
            &getNetwork(Network::Profile::Management));

        Node newNode;
        CPU newNodeCPU;
        BMC newNodeBMC;

        if (ini.exists(nodeSection, "hostname")) {
            newNode.setHostname(ini.getValue(nodeSection, "hostname"));
        } else {
            if (ini.exists(nodeSection, "prefix")) {
                newNode.setPrefix(ini.getValue(nodeSection, "prefix"));
            } else if (genericNode.getPrefix().has_value()) {
                newNode.setPrefix(genericNode.getPrefix());
            } else {
                throw std::runtime_error(fmt::format(
                    "Section node.{} must have a prefix value", nodeCounter));
            }

            LOG_TRACE(
                "{} prefix: {}", nodeSection, newNode.getPrefix().value());

            if (ini.exists(nodeSection, "padding")) {
                newNode.setPadding(
                    std::stoul(ini.getValue(nodeSection, "padding")));
            } else if (genericNode.getPadding().has_value()) {
                newNode.setPadding(genericNode.getPadding().value());
            } else {
                throw std::runtime_error(fmt::format(
                    "Section node.{} must have a padding value", nodeCounter));
            }

            LOG_TRACE(
                "{} padding: {}", nodeSection, newNode.getPrefix().value());

            auto nodeName
                = fmt::format("{}{:0>{}}", newNode.getPrefix().value(),
                    nodeCounter, newNode.getPadding().value());

            newNode.setHostname(nodeName);
        }

        LOG_TRACE("{} hostname: {}", nodeSection, newNode.getHostname());

        if (ini.exists(nodeSection, "node_start_ip")) {
            newNode.setNodeStartIp(boost::asio::ip::make_address(
                ini.getValue(nodeSection, "node_start_ip")));
        } else if (genericNode.getNodeStartIp().has_value()) {
            newNode.setNodeStartIp(genericNode.getNodeStartIp().value());
        } else {
            throw std::runtime_error(
                fmt::format("Section node.{} must have a node_start_ip value",
                    nodeCounter));
        }

        LOG_TRACE("{} start ip: {}", nodeSection,
            newNode.getNodeStartIp()->to_string());

        if (ini.exists(nodeSection, "mac_address")) {
            newNode.setMACAddress(ini.getValue(nodeSection, "mac_address"));
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a node_root_password value",
                nodeCounter));
        }

        LOG_TRACE("{} MAC address: {}", nodeSection, newNode.getMACAddress());

        if (ini.exists(nodeSection, "node_root_password")) {
            newNode.setNodeRootPassword(
                ini.getValue(nodeSection, "node_root_password"));
        } else if (genericNode.getNodeRootPassword().has_value()) {
            newNode.setNodeRootPassword(
                genericNode.getNodeRootPassword().value());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a node_root_password value",
                nodeCounter));
        }

        LOG_TRACE("{} root password: {}", nodeSection,
            newNode.getNodeRootPassword().value());

        if (ini.exists(nodeSection, "sockets")) {
            newNodeCPU.setSockets(
                std::stoul(ini.getValue(nodeSection, "sockets")));
        } else if (genericNode.getCPU().getSockets() != 0) {
            newNodeCPU.setSockets(genericNode.getCPU().getSockets());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a sockets value", nodeCounter));
        }

        LOG_TRACE("{} CPU sockets: {}", nodeSection, newNodeCPU.getSockets());

        if (ini.exists(nodeSection, "cores_per_socket")) {
            newNodeCPU.setCoresPerSocket(
                std::stoul(ini.getValue(nodeSection, "cores_per_socket")));
        } else if (genericNode.getCPU().getCoresPerSocket() != 0) {
            newNodeCPU.setCoresPerSocket(
                genericNode.getCPU().getCoresPerSocket());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a cores_per_socket value",
                nodeCounter));
        }

        LOG_TRACE("{} CPU cores per socket: {}", nodeSection,
            newNodeCPU.getCoresPerSocket());

        if (ini.exists(nodeSection, "threads_per_core")) {
            newNodeCPU.setThreadsPerCore(
                std::stoul(ini.getValue(nodeSection, "threads_per_core")));
        } else if (genericNode.getCPU().getThreadsPerCore() != 0) {
            newNodeCPU.setThreadsPerCore(
                genericNode.getCPU().getThreadsPerCore());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a threads_per_core value",
                nodeCounter));
        }

        LOG_TRACE("{} CPU threads per core: {}", nodeSection,
            newNodeCPU.getThreadsPerCore());

        if (ini.exists(nodeSection, "bmc_address")) {
            newNodeBMC.setAddress(ini.getValue(nodeSection, "bmc_address"));
        } else if (!genericNode.getBMC()->getAddress().empty()) {
            newNodeBMC.setAddress(genericNode.getBMC()->getAddress());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a bmc_address value", nodeCounter));
        }

        LOG_TRACE("{} BMC address: {}", nodeSection, newNodeBMC.getAddress());

        if (ini.exists(nodeSection, "bmc_username")) {
            newNodeBMC.setUsername(ini.getValue(nodeSection, "bmc_username"));

        } else if (!genericNode.getBMC()->getUsername().empty()) {
            newNodeBMC.setUsername(genericNode.getBMC()->getUsername());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a bmc_username value", nodeCounter));
        }

        LOG_TRACE("{} BMC username: {}", nodeSection, newNodeBMC.getUsername());

        if (ini.exists(nodeSection, "bmc_password")) {
            newNodeBMC.setPassword(ini.getValue(nodeSection, "bmc_password"));

        } else if (!genericNode.getBMC()->getPassword().empty()) {
            newNodeBMC.setPassword(genericNode.getBMC()->getPassword());
        } else {
            throw std::runtime_error(fmt::format(
                "Section node.{} must have a bmc_password value", nodeCounter));
        }

        LOG_TRACE("{} BMC password: {}", nodeSection, newNodeBMC.getPassword());

        if (ini.exists(nodeSection, "bmc_serialport")) {
            newNodeBMC.setSerialPort(
                std::stoul(ini.getValue(nodeSection, "bmc_serialport")));
        } else if (genericNode.getBMC()->getSerialPort() != 0) {
            newNodeBMC.setSerialPort(genericNode.getBMC()->getSerialPort());
        } else {
            throw std::runtime_error(
                fmt::format("Section node.{} must have a bmc_serialport value",
                    nodeCounter));
        }

        LOG_TRACE(
            "{} BMC serial port: {}", nodeSection, newNodeBMC.getSerialPort());

        if (ini.exists(nodeSection, "bmc_serialspeed")) {
            newNodeBMC.setSerialSpeed(
                std::stoul(ini.getValue(nodeSection, "bmc_serialspeed")));
        } else if (genericNode.getBMC()->getSerialSpeed() != 0) {
            newNodeBMC.setSerialSpeed(genericNode.getBMC()->getSerialSpeed());
        } else {
            throw std::runtime_error(
                fmt::format("Section node.{} must have a bmc_serialspeed value",
                    nodeCounter));
        }

        LOG_TRACE("{} BMC serial speed: {}", nodeSection,
            newNodeBMC.getSerialSpeed());

        newNode.setCPU(newNodeCPU);
        newNode.setBMC(newNodeBMC);
        newNode.setOS(nodeOS);

        connection.setMAC(newNode.getMACAddress());
        connection.setAddress(newNode.getNodeStartIp().value());
        newNode.setConnection(nodeConnections);

        addNode(newNode);
        nodeCounter++;
    }

    /* Bad and old data - @TODO Must improve */
    nodePrefix = genericNode.getPrefix().value();
    nodePadding = genericNode.getPadding().value();
    nodeStartIP = genericNode.getNodeStartIp().value();
    nodeRootPassword = genericNode.getNodeRootPassword().value();
}