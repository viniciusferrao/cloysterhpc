#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
#include <optional>
#include <memory>
#include <filesystem>

#include "headnode.h"
#include "node.h"
#include "network.h"
#include "types.h"
#include "services/timezone.h"

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
    bool isFirewall() const;
    void setFirewall(bool firewall);
    SELinuxMode getSELinux() const;
    void setSELinux(SELinuxMode);
    const Timezone& getTimezone() const;
    void setTimezone(const std::string& tz);
    const std::string& getLocale() const;
    void setLocale(const std::string &locale);
    const std::string& getDomainName() const;
    void setDomainName(const std::string &domainName);
    std::list<Network>& getNetworks();
    Network& getNetwork(Network::Profile profile);
    void addNetwork();
    void addNetwork(Network::Profile profile);
    void addNetwork(Network::Profile profile, Network::Type, const std::string&,
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
    void addNode(std::string_view, const Network& network, const std::string&, const std::string&,
                 std::string_view, std::string_view, std::string_view);

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
    std::string nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;
    QueueSystem queueSystem;
    Postfix postfix;
    bool remoteAccess {};

    //Cluster();
    //~Cluster();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
