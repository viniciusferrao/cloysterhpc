#ifndef _HPP_CLUSTER_
#define _HPP_CLUSTER_

#include <string> /* std::string */

#include "types.hpp"

class Cluster {
private:
    void setTimezone (std::string);
    void setLocale (std::string);
    void setFQDN (std::string);
    void enableFirewall (void);
    void disableFirewall (void);
    int setSELinuxMode (std::string);
    void installRequiredPackages (void);
    void setupRepositories (void);
    void installProvisioningServices (void);

public:
    bool firewall;
    bool selinux;
    std::string timezone;
    std::string locale;
    std::string hostname;
    std::string domainname;
    std::string fqdn;
    NETWORK service;
    NETWORK management;
    NETWORK application;
    std::string interfaceExternal;
    std::string interfaceInternal;
    std::string interfaceInternalNetwork;
    std::string interfaceInternalIP;
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
    std::string ibStack;
    QUEUESYSTEM queueSystem;
    POSTFIX postfix;
    bool updateSystem;
    bool remoteAccess;

    void install (void);
};

#endif /* _HPP_CLUSTER_ */