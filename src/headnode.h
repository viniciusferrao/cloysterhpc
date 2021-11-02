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
    std::string m_hostname;
public:
    const std::string &getHostname() const;
    void setHostname(const std::string &hostname);
    const std::string &getFQDN() const;
    void setFQDN(const std::string &fqdn);
    const OS &getOS() const;
    void setOS(const OS &os);

private:
    std::string m_fqdn;
    OS m_os;
    std::vector<Connection> m_externalConnection;

private:
    std::string fetchValue (const std::string& line);

public:
    int setOS ();
    void printOS ();
    int checkSupportedOS ();
};

#endif /* HEADNODE_H */
