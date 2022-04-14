#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
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

class Cluster {
public:
    enum class SELinuxMode { Permissive, Enforcing, Disabled };
    enum class Provisioner { xCAT };
    enum class Directory { None, FreeIPA };

private:
    std::string_view m_name;
    std::string_view m_companyName;
    std::string_view m_adminMail;
    Headnode m_headnode;
    Provisioner m_provisioner;
    std::optional<OFED> m_ofed;
    std::optional<std::unique_ptr<QueueSystem>> m_queueSystem{};
    std::optional<Postfix> m_mailSystem{};
    std::vector<Node> m_nodes;

    bool m_firewall{};
    SELinuxMode m_selinux{}; /* Control nodes SELinux settings */
    Timezone m_timezone;
    std::string m_locale; /* Default locale cluster wide */
    std::string m_domainName;

    std::list<Network> m_network;

    bool m_updateSystem {};
    std::filesystem::path m_isoPath;

public:
    // If we want to change things inside the headnode it must be passed as a
    // reference, or else a simple setter will fail.
    Headnode& getHeadnode();
    [[nodiscard]] const Headnode& getHeadnode() const;
    const std::string_view& getName() const;
    void setName(const std::string_view &name);
    const std::string_view& getCompanyName() const;
    void setCompanyName(const std::string_view &companyName);
    const std::string_view& getAdminMail() const;
    void setAdminMail(const std::string_view &adminMail);
    bool isFirewall() const;
    void setFirewall(bool firewall);
    SELinuxMode getSELinux() const;
    void setSELinux(SELinuxMode);
    Timezone& getTimezone();
    void setTimezone(const std::string& tz);
    const std::string& getLocale() const;
    void setLocale(const std::string &locale);
    const std::string& getDomainName() const;
    void setDomainName(const std::string &domainName);
    std::list<Network>& getNetworks();
    Network& getNetwork(Network::Profile profile);
    void addNetwork();
    void addNetwork(Network::Profile profile);
    void addNetwork(Network::Profile profile, Network::Type type);
    void addNetwork(Network::Profile profile, Network::Type, const std::string&,
                    const std::string&, const std::string&, const uint16_t&,
                    const std::string&, const std::vector<std::string>&);
    void addNetwork(Network&& network);

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

    const std::filesystem::path &getISOPath() const;
    void setISOPath(const std::filesystem::path &isoPath);

    const std::vector<Node>& getNodes() const;
    void addNode(OS& os, CPU& cpu, std::string_view hostname, const Network& network,
                 std::string_view mac, const std::string& address);
    void addNode(OS& os, CPU& cpu, std::string_view hostname, const Network& network,
                 std::string_view mac, const std::string& address, BMC& bmc);

#ifdef _DEBUG_
    void printNetworks(const std::list<Network>&);
#endif

    /* TODO: Refactor all those leftovers from legacy C version */
    std::string xCATDynamicRangeStart;
    std::string xCATDynamicRangeEnd;
    std::string directoryAdminPassword;
    std::string directoryManagerPassword;
    bool directoryDisableDNSSEC {};
    std::string nodePrefix;
    size_t nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;
    bool remoteAccess {};

    //Cluster();
    //~Cluster();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
