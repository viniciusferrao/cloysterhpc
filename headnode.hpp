#ifndef _HPP_HEADNODE_
#define _HPP_HEADNODE_

#include <string> /* std::string */

#include "types.hpp"

enum class Arch {
    x86_64,
    ppc64le
};

enum class Family {
    Linux,
    Darwin // Development reasons, not really supported.
};

enum class Platform {
    el8
};

enum class Distro {
    RHEL,
    OL
};

class Headnode {
private:
    std::string fetchValue (std::string line);

public:
    Arch arch;
    struct {
        Family family;
        Platform platform;
        Distro distro;
        std::string kernel;
        unsigned majorVersion;
        unsigned minorVersion;
    } os;
    struct {
        std::string hostname;
        std::string domainname;
        IP ip;
    } network;

    int setOS (void);
    int checkSupportedOS (void);   
};

#endif /* _HPP_HEADNODE_ */