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
protected:
    Headnode m_headnode;

private:
    void setTimezone (std::string);
    void setLocale (std::string);
    void setFQDN (std::string);
    void enableFirewall (void);
    void disableFirewall (void);
    int setSELinuxMode (std::string);
    void systemUpdate (void);
    void installRequiredPackages (void);
    void setupRepositories (void);
    void installProvisioningServices (void);
    void setupTimeService (void);
    void setupSLURM (void);
    void setupInfiniband (void);
    void disableNetworkManagerDNSOverride (void); // This should be on Network
    void setupInternalNetwork (void); // This should be on Network
    void setupNetworkFileSystem (void);

public:
    bool firewall; // Cluster-wide firewall settings
    bool selinux; // Cluster-wide SELinux settings
    std::string timezone;
    std::string locale; // Default locale cluster wide.
    std::string domainname;


    Network service;
    Network management;
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

    Cluster (const Headnode&);
    void install (void);
};

#endif /* CLUSTER_H */
