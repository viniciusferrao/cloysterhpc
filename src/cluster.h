/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CLUSTER_H_
#define CLOYSTERHPC_CLUSTER_H_

#include <string>
#include <optional>
#include <memory>
#include <filesystem>

#include "headnode.h"
#include "node.h"
#include "network.h"
#include "ofed.h"
#include "services/timezone.h"
#include "queuesystem/slurm.h"
#include "queuesystem/pbs.h"
#include "mailsystem/postfix.h"
#include "diskImage.h"

class Cluster {
public:
    enum class SELinuxMode { Permissive, Enforcing, Disabled };
    enum class Provisioner { xCAT };

private:
    std::string m_name;
    std::string m_companyName;
    std::string m_adminMail;
    Headnode m_headnode;
    Provisioner m_provisioner{Provisioner::xCAT};
    std::optional<OFED> m_ofed;
    std::optional<std::unique_ptr<QueueSystem>> m_queueSystem{};
    std::optional<Postfix> m_mailSystem{};
    std::vector<Node> m_nodes;

    bool m_firewall{false};
    SELinuxMode m_selinux{SELinuxMode::Disabled};
    Timezone m_timezone;
    std::string m_locale; /* Default locale cluster wide */
    std::string m_domainName;

    std::list<std::unique_ptr<Network>> m_network;

    bool m_updateSystem{false};
    DiskImage m_diskImage;

public:
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
    [[nodiscard]] const std::string& getLocale() const;
    void setLocale(const std::string& locale);
    [[nodiscard]] const std::string& getDomainName() const;
    void setDomainName(const std::string& domainName);
    std::list<std::unique_ptr<Network>>& getNetworks();
    Network& getNetwork(Network::Profile profile);
    void addNetwork();
    void addNetwork(Network::Profile profile);
    void addNetwork(Network::Profile profile, Network::Type type);
    void addNetwork(Network::Profile profile, Network::Type, const std::string&,
                    const std::string&, const std::string&, const uint16_t&,
                    const std::string&, const std::vector<std::string>&);
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
    void setMailSystem(Postfix::Profile profile);

    const std::filesystem::path& getDiskImage() const;
    void setDiskImage(const std::filesystem::path& diskImagePath);

    // TODO: Add std::optional to BMC with std::nullopt as default initializer
    [[nodiscard]] const std::vector<Node>& getNodes() const;
    void addNode(std::string_view hostname, OS& os, CPU& cpu,
                 std::list<Connection>&& connections, BMC& bmc);
    void addNode(std::string_view hostname, OS& os, CPU& cpu,
                 std::list<Connection>&& connections);

#ifndef NDEBUG
    void printNetworks(const std::list<std::unique_ptr<Network>>& networks) const;
    void printConnections();
    void printData();
    void fillTestData();
#endif

    /* TODO: Refactor all those leftovers from legacy C version */
    std::size_t nodeQuantity;
    std::string nodePrefix;
    std::size_t nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;

};

#endif // CLOYSTERHPC_CLUSTER_H_
