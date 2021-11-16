#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
#include <optional>
#include <filesystem>

#include "headnode.h"
#include "node.h"
#include "network.h"
#include "types.h"

struct SLURM {
    std::string partition;
};

struct PBS {
    enum class DefaultPlace { Shared, Scatter };
    DefaultPlace defaultPlace;
};

struct QueueSystem {
    std::string name;
    std::optional<SLURM> slurm;
    std::optional<PBS> pbs;
};

struct PostfixRelay {
    std::string hostname;
    uint16_t port;
};

struct PostfixSASL {
    std::string hostname;
    uint16_t port;
    std::string username;
    std::string password;
};

struct Postfix {
    bool enable;
    enum class ProfileId { Relay, SASL };
    ProfileId profileId;
    std::optional<PostfixRelay> relay;
    std::optional<PostfixSASL> sasl;
};

class Cluster {
public:
    enum class SELinuxMode { Permissive, Enforcing, Disabled };
    enum class Provisioner { xCAT };
    enum class Directory { None, FreeIPA };
    enum class OFED { None, Inbox, Mellanox, Oracle };
    enum class QueueSystem { None, SLURM, PBS };

private:
    //std::unique_ptr<Headnode> m_headnode;
    Headnode m_headnode;
    Provisioner m_provisioner;
    OFED m_ofed;
    std::vector<Node> m_nodes;

    bool m_firewall {};
    SELinuxMode m_selinux {}; /* Control nodes SELinux settings */
    std::string m_timezone;
    std::string m_locale; /* Default locale cluster wide */
    std::string m_domainName;
    struct {
        std::vector<std::shared_ptr<Network>> external;
        std::vector<std::shared_ptr<Network>> management;
        std::vector<std::shared_ptr<Network>> service;
        std::vector<std::shared_ptr<Network>> application;
    } m_network;
    bool m_updateSystem {};
    std::filesystem::path m_isoPath;

public:
    //const std::unique_ptr<Headnode>& getHeadnode() const;
    const Headnode getHeadnode() const;
    bool isFirewall() const;
    void setFirewall(bool firewall);
    SELinuxMode getSELinux() const;
    void setSELinux(SELinuxMode);
    const std::string& getTimezone() const;
    void setTimezone(const std::string &timezone);
    const std::string& getLocale() const;
    void setLocale(const std::string &locale);
    const std::string& getDomainName() const;
    void setDomainName(const std::string &domainName);
    const std::vector<std::shared_ptr<Network>>& getNetwork(Network::Profile) const;
    void addNetwork(Network::Profile, Network::Type, const std::string&,
                    const std::string&, const std::string&, const uint16_t&,
                    const std::string&, const std::vector<std::string>&);

    bool isUpdateSystem() const;
    void setUpdateSystem(bool);

    Provisioner getProvisioner() const;
    void setProvisioner(Provisioner);

    OFED getOFED() const;
    void setOFED(OFED);

    const std::filesystem::path &getISOPath() const;
    void setISOPath(const std::filesystem::path &isoPath);

    const std::vector<Node>& getNodes() const;
    void addNode(std::string_view, const std::string&, const std::string&,
                 std::string_view, std::string_view, std::string_view);

#ifdef _DEBUG_
    void printNetworks();
#endif

    /* TODO: Refactor all those leftovers from legacy C version */
    std::string xCATDynamicRangeStart;
    std::string xCATDynamicRangeEnd;
    std::string directoryAdminPassword;
    std::string directoryManagerPassword;
    bool directoryDisableDNSSEC {};
    std::string nodePrefix;
    std::string nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;
    QueueSystem queueSystem;
    Postfix postfix;
    bool remoteAccess {};

    Cluster ();
    ~Cluster ();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
