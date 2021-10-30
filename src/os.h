#ifndef OS_H
#define OS_H

#include <string>

/* Darwin added for development reasons, not really supported. */
class OS {
public:
    enum class Arch { x86_64, ppc64le };
    enum class Family { Linux, Darwin }; 
    enum class Platform { el8 };
    enum class Distro { RHEL, OL };

    Arch arch;
    Family family;
    Platform platform;
    Distro distro;
    std::string kernel;
    unsigned majorVersion;
    unsigned minorVersion;
};

#endif /* OS_H */
