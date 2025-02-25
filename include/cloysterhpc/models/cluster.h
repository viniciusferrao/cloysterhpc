/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CLUSTER_H_
#define CLOYSTERHPC_CLUSTER_H_

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <cloysterhpc/dbus_client.h>
#include <cloysterhpc/diskImage.h>
#include <cloysterhpc/mailsystem/postfix.h>
#include <cloysterhpc/models/headnode.h>
#include <cloysterhpc/models/node.h>
#include <cloysterhpc/models/queuesystem.h>
#include <cloysterhpc/network.h>
#include <cloysterhpc/ofed.h>
#include <cloysterhpc/services/locale.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/timezone.h>

/**
 * @class Cluster
 * @brief Represents a cluster environment.
 *
 * This class manages various configuration settings and resources for a cluster
 * environment, including headnode, nodes, networks, provisioner, timezone,
 * locale, and more.
 */

namespace cloyster::models {

/**
 * @brief Represents the cluster state and configuration
 */
class Cluster {
public:
    /**
     * @enum SELinuxMode
     * @brief Enumeration for SELinux modes.
     *
     */
    enum class SELinuxMode { Permissive, Enforcing, Disabled };

    // @TODO: This class should not know about DBusClient

    /**
     * @enum Provisioner
     * @brief Enumeration for cluster provisioners.
     */
    enum class Provisioner { xCAT };

private:
    std::string m_name;
    std::string m_companyName;
    std::string m_adminMail;
    Headnode m_headnode;
    Provisioner m_provisioner { Provisioner::xCAT };
    std::optional<OFED> m_ofed;
    std::optional<std::unique_ptr<QueueSystem>> m_queueSystem {};
    std::optional<Postfix> m_mailSystem {};
    std::vector<Node> m_nodes;
    std::shared_ptr<DBusClient> m_systemdBus;

    bool m_firewall { false };
    SELinuxMode m_selinux { SELinuxMode::Disabled };
    Timezone m_timezone;
    Locale m_locale; /* Default locale cluster wide */
    std::string m_domainName;

    std::list<std::unique_ptr<Network>> m_network;

    bool m_updateSystem { false };
    DiskImage m_diskImage;

public:
    Cluster();

    [[nodiscard]] Headnode& getHeadnode();
    [[nodiscard]] const Headnode& getHeadnode() const;

    [[nodiscard]] std::string_view getName() const;
    void setName(std::string_view name);
    [[nodiscard]] std::string_view getCompanyName() const;
    void setCompanyName(std::string_view companyName);
    [[nodiscard]] std::string_view getAdminMail() const;
    void setAdminMail(std::string_view adminMail);
    [[nodiscard]] bool isFirewall() const;
    void setFirewall(bool firewall);
    [[nodiscard]] SELinuxMode getSELinux() const;
    void setSELinux(SELinuxMode);
    [[nodiscard]] Timezone& getTimezone();
    void setTimezone(const std::string& tz);
    [[nodiscard]] const Locale& getLocale() const;
    void setLocale(const Locale& locale);
    void setLocale(const std::string& locale);
    [[nodiscard]] const std::string getDomainName() const;
    void setDomainName(const std::string& domainName);
    std::list<std::unique_ptr<Network>>& getNetworks();
    Network& getNetwork(Network::Profile profile);

    std::shared_ptr<DBusClient> getDaemonBus();

    /**
     * @brief Add a new network to the cluster.
     *
     */
    void addNetwork();

    /**
     * @brief Add a new network to the cluster.
     *
     * @param profile Profile of the network.
     */
    void addNetwork(Network::Profile profile);

    /**
     * @brief Add a new network to the cluster.
     *
     * @param profile Profile of the network.
     * @param type Type of the network.
     */
    void addNetwork(Network::Profile profile, Network::Type type);

    /**
     * @brief Adds a new network to the cluster with formatted nameservers.
     *
     * This function adds a new network to the cluster with the provided
     * configuration, including IP address, subnet mask, gateway, VLAN, domain
     * name, and formatted nameservers.
     *
     * @param profile Profile of the network.
     * @param type Type of the network.
     * @param ip IP address of the network.
     * @param subnetMask Subnet mask of the network.
     * @param gateway Gateway address of the network.
     * @param vlan VLAN ID of the network.
     * @param domainName Domain name of the network.
     * @param nameserver List of nameservers in address format.
     */
    void addNetwork(Network::Profile profile, Network::Type, const std::string&,
        const std::string&, const std::string&, const uint16_t&,
        const std::string&, const std::vector<address>&);

    /**
     * @brief Adds a new network to the cluster with formatted nameservers.
     *
     * This function adds a new network to the cluster with the provided
     * configuration, including IP address, subnet mask, gateway, VLAN, domain
     * name, and formatted nameservers.
     *
     * @param profile Profile of the network.
     * @param type Type of the network.
     * @param ip IP address of the network.
     * @param subnetMask Subnet mask of the network.
     * @param gateway Gateway address of the network.
     * @param vlan VLAN ID of the network.
     * @param domainName Domain name of the network.
     * @param nameserver List of nameservers in string format.
     */
    void addNetwork(Network::Profile profile, Network::Type, const std::string&,
        const std::string&, const std::string&, const uint16_t&,
        const std::string&, const std::vector<std::string>&);

    /**
     * @brief Add a new network to the cluster.
     *
     * @param network The network.
     */
    void addNetwork(std::unique_ptr<Network>&& network);

    bool isUpdateSystem() const;
    void setUpdateSystem(bool);

    Provisioner getProvisioner() const;
    void setProvisioner(Provisioner);

    std::optional<OFED> getOFED() const;
    void setOFED(OFED::Kind kind);

    std::optional<std::unique_ptr<QueueSystem>>& getQueueSystem();
    void setQueueSystem(QueueSystem::Kind kind);

    std::optional<Postfix>& getMailSystem();
    void setMailSystem(
        Postfix::Profile profile, std::shared_ptr<services::BaseRunner> runner);

    const DiskImage& getDiskImage() const;
    void setDiskImage(const std::filesystem::path& diskImagePath);

    // TODO: Add std::optional to BMC with std::nullopt as default initializer
    [[nodiscard]] const std::vector<Node>& getNodes() const;

    /**
     * @brief Adds a new node to the cluster.
     *
     * This function adds a new node to the cluster with the provided hostname,
     * operating system, CPU, connections, and BMC configuration.
     *
     * @param hostname The hostname of the node.
     * @param os Reference to the operating system object.
     * @param cpu Reference to the CPU object.
     * @param connections Rvalue reference to a list of connections.
     * @param bmc Reference to the BMC object.
     */
    void addNode(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections, BMC& bmc);

    /**
     * @brief Adds a new node to the cluster without BMC configuration.
     *
     * This function adds a new node to the cluster with the provided hostname,
     * operating system, CPU, and connections.
     *
     * @param hostname The hostname of the node.
     * @param os Reference to the operating system object.
     * @param cpu Reference to the CPU object.
     * @param connections Rvalue reference to a list of connections.
     */
    void addNode(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections);

    /**
     * @brief Adds an existing node to the cluster.
     *
     * This function adds an existing node to the cluster.
     *
     * @param node The node object to be added.
     */
    void addNode(Node node);

    /**
     * @brief Fills cluster data from the specified answer file.
     *
     * @param answerfilePath Path to the answer file.
     */
    void fillData(const std::filesystem::path& answerfilePath);

    void dumpData(const std::filesystem::path& answerfilePath);

#ifndef NDEBUG
    void printNetworks(
        const std::list<std::unique_ptr<Network>>& networks) const;
    void printConnections();
    void printData();
    void fillTestData();
#endif

    /* TODO: Refactor all those leftovers from legacy C version */
    std::size_t nodeQuantity = 0;
    std::string nodePrefix;
    std::size_t nodePadding = 0;
    address nodeStartIP;
    std::string nodeRootPassword;
};

}; // namespace cloyster::models

#endif // CLOYSTERHPC_CLUSTER_H_
