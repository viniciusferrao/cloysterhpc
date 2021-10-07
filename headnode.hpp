#ifndef _HPP_HEADNODE_
#define _HPP_HEADNODE_

#include <string> /* std::string */
#include <vector>

#include <arpa/inet.h>

#include "types.hpp"

enum class Arch { x86_64, ppc64le };

class Network {
public:
    enum class Profile { External, Management, Service, Application };
    enum class Type { Ethernet, Infiniband };

private:
    Profile m_profile;
    Type m_type;
    struct in_addr m_addr;

public:
    struct {
        std::string interface;
        uint16_t mtu;
    } l2;

    struct {
        uint32_t ip;
        uint32_t mask;
        uint32_t gateway;
    } l3;

    std::string hostname;
    std::string domainname;
    std::string fqdn;
    std::vector<std::string> nameserver;

    int setProfile (std::string);
    std::string getProfile (void);

    int setType (Type);
    Type getType (void);

    int setIPAddress (std::string);
    std::string getIPAddress (void);

};

class OS {
public:
    // Darwin added for development reasons, not really supported.
    enum class Family { Linux, Darwin };
    enum class Platform { el8 };
    enum class Distro { RHEL, OL };

    Family family;
    Platform platform;
    Distro distro;
    std::string kernel;
    unsigned majorVersion;
    unsigned minorVersion;

};

class Headnode {
private:
    std::string fetchValue (std::string line);

public:
    Arch arch;
    OS os;
    //std::vector<Network> network;
    Network network;

    int setOS (void);
    int checkSupportedOS (void);   
};

#endif /* _HPP_HEADNODE_ */
