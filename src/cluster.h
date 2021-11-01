#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
#include <optional>

#include "headnode.h"
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
    Headnode* m_headnode;

    bool firewall; // Cluster-wide firewall settings
    bool selinux; // Cluster-wide SELinux settings
    std::string timezone;
    std::string locale; // Default locale cluster wide.
    std::string domainname;

    Network external;
    Network management;
    Network service;
    Network application;

    std::string xCATDynamicRangeStart;
    std::string xCATDynamicRangeEnd;
    std::string directoryAdminPassword;
    std::string directoryManagerPassword;
    bool directoryDisableDNSSEC;
    std::string nodePrefix;
    std::string nodePadding;
    std::string nodeStartIP;
    std::string nodeRootPassword;
    std::string nodeISOPath;
    std::string ibStack; /* Refactor */
    QueueSystem queueSystem;
    Postfix postfix;
    bool updateSystem;
    bool remoteAccess;

    Cluster ();
    Cluster (Headnode&);
    ~Cluster ();

#ifdef _DEBUG_
    void printData ();
    void fillTestData ();
#endif
};

#endif /* CLUSTER_H */
