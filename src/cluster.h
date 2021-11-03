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
public: /* Must be private */
    std::unique_ptr<Headnode> m_headnode;

    // Cluster-wide m_firewall settings
private:
    bool m_firewall{};
    bool m_selinux{}; // Cluster-wide SELinux settings
    std::string m_timezone;
    std::string m_locale; // Default m_locale cluster wide.
    std::string m_domainName;
    /* TODO: Better networking, this is just bad */
    struct {
        Network external;
        Network management;
        Network service;
        Network application;
    } m_network;

public:
    bool isFirewall() const;
    void setFirewall(bool firewall);
    bool isSELinux() const;
    void setSELinux(bool selinux);
    const std::string& getTimezone() const;
    void setTimezone(const std::string &timezone);
    const std::string& getLocale() const;
    void setLocale(const std::string &locale);
    const std::string& getDomainName() const;
    void setDomainName(const std::string &domainName);
    const Network getNetwork(Network::Profile) const;
    void setNetwork(Network::Profile, Network::Type, std::string,
                    std::string, std::string, uint16_t, std::string,
                    std::vector<std::string>);

    /* TODO: Refactor all those leftovers from legacy C version */
    std::string xCATDynamicRangeStart;
    std::string xCATDynamicRangeEnd;
    std::string directoryAdminPassword;
    std::string directoryManagerPassword;
    bool directoryDisableDNSSEC{};
    std::string nodePrefix;
    std::string nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;
    std::string nodeISOPath;
    std::string ibStack; /* Refactor */
    QueueSystem queueSystem;
    Postfix postfix;
    bool updateSystem{};
    bool remoteAccess{};

    Cluster ();
    ~Cluster ();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
