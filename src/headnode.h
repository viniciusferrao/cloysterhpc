#ifndef HEADNODE_H
#define HEADNODE_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

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
    OS m_os;
    std::string m_hostname;
    std::string m_fqdn;
    std::vector<Connection> m_connection;

private:
    void discoverNames();

public:
    Headnode();

    [[nodiscard]] const OS& getOS() const;

    [[nodiscard]] const std::string& getHostname() const;
    void setHostname(const std::string& hostname);

    [[nodiscard]] const std::string& getFQDN() const;
    void setFQDN(const std::string& fqdn);

    //const std::unique_ptr<Connection>& getConnection() const;
    [[nodiscard]] const std::vector<Connection>& getConnections() const;
    void addConnection(const std::shared_ptr<Network>&, const std::string&,
                       const std::string&);

    [[nodiscard]] const Connection& getConnection(Network::Profile) const;
};

#endif /* HEADNODE_H */
