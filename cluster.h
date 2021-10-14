#ifndef CLUSTER_H
#define CLUSTER_H

#include <string> /* std::string */
#include <optional>

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
    bool firewall; // Perhaps this should be on Headnode instead
    bool selinux; // Perhaps this should be on Headnode instead
    std::string timezone;
    std::string locale; // Perhaps this should be on Headnode instead
    std::string hostname; // Definitely on Headnode
    std::string domainname;
    std::string fqdn; // FQDN of the Headnode

#if 0
    NETWORK service; // This should describle available nets but not addresses
    NETWORK management;
    NETWORK application;
#endif

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

    void install (void);
};

#endif /* CLUSTER_H */
