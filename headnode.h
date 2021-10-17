#ifndef HEADNODE_H
#define HEADNODE_H

#include <string> /* std::string */
#include <vector>
#include <optional>

#include "types.h"
#include "network.h"
#include "connection.h"
#include "os.h"

class Headnode {
private:
    std::string fetchValue (std::string line);

public:
    OS os;
    /* To be removed */
    std::vector<Network> externalNetwork;
    std::vector<Network> managementNetwork;
    std::optional<std::vector<Network>> serviceNetwork;
    std::optional<std::vector<Network>> applicationNetwork;
    std::optional<std::vector<Network>> otherNetwork;
    std::string domainname;
    /* End of removal */



    std::vector<Connection> externalConnection;

    std::string timezone;
    std::string locale;
    std::string hostname;
    std::string fqdn;
    std::vector<std::string> nameserver;

    int setOS (void);
    void printOS (void);
    int checkSupportedOS (void);   
};

#endif /* HEADNODE_H */
