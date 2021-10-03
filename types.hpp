/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef _H_TYPES_
#define _H_TYPES_

#include <string> /* std::string */

typedef enum {
    X86_64
} ARCH;

typedef struct {
    uint32_t address;
    uint32_t netmask;
    uint32_t gateway;
} IP;

typedef struct {
    bool enable;
    char* interface;
    IP ip;
} NETWORK;

typedef struct {
    char *partition;
} QUEUESYSTEM_SLURM;

typedef struct {
    char *defaultPlace;
} QUEUESYSTEM_PBS;

typedef struct {
    char* name;
    union {
        QUEUESYSTEM_SLURM slurm;
        QUEUESYSTEM_PBS pbs;
    };
} QUEUESYSTEM;

typedef struct {
    char* hostname;
    uint16_t port;
} POSTFIX_RELAY;

typedef struct {
    char* hostname;
    uint16_t port;
    char* username;
    char* password;
} POSTFIX_SASL;

typedef struct {
    bool enable;
    unsigned profileId;
    union {
        POSTFIX_RELAY relay;
        POSTFIX_SASL sasl;
    };
} POSTFIX;

#if 0
/* We describe the entire cluster in a single struct and this is a bada data 
 * model. We need to adapt this to have smaller structs merged in a single
 * struct at the end.
 * Unions may be used in mutually exclusive selections, for instance the queue
 * system and the Infiniband stack.
 */
typedef struct {
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
} CLUSTER;
#endif

#endif /* _H_TYPES_ */
