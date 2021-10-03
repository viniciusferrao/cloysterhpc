#ifndef _HPP_CLUSTER_
#define _HPP_CLUSTER_

#include <string> /* std::string */

#include "types.hpp"

class Cluster {
private:
    void setTimezone (std::string timezone);

public:
    char *timezone;
    char *locale;
    char *hostname;
    char *domainname;
    char *fqdn;
    NETWORK service;
    NETWORK management;
    NETWORK application;
    char *interfaceExternal;
    char *interfaceInternal;
    char *interfaceInternalNetwork;
    char *interfaceInternalIP;
    char *xCATDynamicRangeStart;
    char *xCATDynamicRangeEnd;
    char *directoryAdminPassword;
    char *directoryManagerPassword;
    bool directoryDisableDNSSEC;
    char *nodePrefix;
    char *nodePadding;
    char *nodeStartIP;
    char *nodeRootPassword;
    char *nodeISOPath;
    char *ibStack;
    QUEUESYSTEM queueSystem;
    POSTFIX postfix;
    bool updateSystem;
    bool remoteAccess;

    void install (void);
};

#endif /* _HPP_CLUSTER_ */