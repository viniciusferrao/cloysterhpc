#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
#include <optional>

#include "headnode.h"
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

private:
    std::unique_ptr<Headnode> m_headnode; /* Headnode m_headnode; */

    bool m_firewall {};
    SELinuxMode m_selinux; /* Control nodes SELinux settings */
    std::string m_timezone;
    std::string m_locale; /* Default locale cluster wide */
    std::string m_domainName;
    struct {
        std::vector<std::unique_ptr<Network>> external;
        std::vector<std::unique_ptr<Network>> management;
        std::vector<std::unique_ptr<Network>> service;
        std::vector<std::unique_ptr<Network>> application;
    } m_network;

public:
    const std::unique_ptr<Headnode>& getHeadnode() const;
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
    const std::vector<std::unique_ptr<Network>>& getNetwork(
                                                    Network::Profile) const;
    void addNetwork(Network::Profile, Network::Type, const std::string&,
                    const std::string&, const std::string&, const uint16_t&,
                    const std::string&, const std::vector<std::string>&);
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
    std::string nodeISOPath;
    std::string ibStack; /* Refactor */
    QueueSystem queueSystem;
    Postfix postfix;
    bool updateSystem {};
    bool remoteAccess {};

    Cluster ();
    ~Cluster ();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
