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
    std::string m_fqdn;
    OS m_os;
    std::vector<Connection> m_connection;

public:
    const std::string& getHostname() const;
    void setHostname(const std::string &hostname);
    const std::string& getFQDN() const;
    void setFQDN(const std::string &fqdn);
    const OS& getOS() const;
    void setOS(const OS &os);

private:
    std::string fetchValue (const std::string& line);
    const std::string discoverHostname();

public:
    Headnode();

    //const std::unique_ptr<Connection>& getConnection() const;
    const std::vector<Connection>& getConnections() const;
    void addConnection(const std::shared_ptr<Network>&, const std::string&,
                       const std::string&);

    int discoverOS ();
    void printOS ();
    int checkSupportedOS ();
};

#endif /* HEADNODE_H */
