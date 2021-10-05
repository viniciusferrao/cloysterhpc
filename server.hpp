#ifndef _HPP_SERVER_
#define _HPP_SERVER_

#include <string> /* std::string */

#include "types.hpp"

class Server {
private:
    std::string fetchValue (std::string line);

public:
    ARCH arch;
    struct {
        std::string family;
        std::string platform;
        std::string id;
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

#endif /* _HPP_SERVER_ */