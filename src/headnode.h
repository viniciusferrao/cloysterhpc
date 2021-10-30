#ifndef HEADNODE_H
#define HEADNODE_H

#include <string> /* std::string */
#include <vector>
#include <optional>

#include "server.h"
#include "types.h"
#include "network.h"
#include "connection.h"
#include "os.h"

class Headnode : public Server {
private:
    std::string fetchValue (const std::string& line);

public:
    OS os;

    std::vector<Connection> external;

    std::string hostname;
    std::string fqdn;

    int setOS ();
    void printOS ();
    int checkSupportedOS ();
};

#endif /* HEADNODE_H */
