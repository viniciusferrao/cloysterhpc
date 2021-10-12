#ifndef _HPP_HEADNODE_
#define _HPP_HEADNODE_

#include <string> /* std::string */
#include <vector>
#include <optional>

#include <arpa/inet.h>

#include "types.hpp"

enum class Arch { x86_64, ppc64le };

class Network {
public:
    enum class Profile { External, Management, Service, Application };
    enum class Type { Ethernet, Infiniband };

    std::string hostname;
    std::string domainname;
    std::string fqdn;
    std::vector<std::string> nameserver;

private:
    Profile m_profile;
    Type m_type;
    struct ifaddrs *m_ifaddr;
    struct in_addr m_address;
    struct in_addr m_subnetmask;
    struct in_addr m_gateway;

public:
    Network ();

    void setProfile (Profile);
    Profile getProfile (void);

    void setType (Type);
    Type getType (void);

    int setInterfaceName (void);
    void printInterfaceName (void);

    int setIPAddress (std::string, std::string);
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
    std::vector<Network> externalNetwork;
    std::vector<Network> managementNetwork;
    std::optional<std::vector<Network>> serviceNetwork;
    std::optional<std::vector<Network>> applicationNetwork;
    std::optional<std::vector<Network>> otherNetwork;

    std::string timezone;
    std::string locale;
    std::string hostname;
    std::string domainname;
    std::string fqdn;
    std::vector<std::string> nameserver;

    int setOS (void);
    void printOS (void);
    int checkSupportedOS (void);   
};

#endif /* _HPP_HEADNODE_ */
