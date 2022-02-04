#ifndef HEADNODE_H
#define HEADNODE_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include <string>
#include <list>
#include <memory>

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
    std::list<Connection> m_connection;

private:
    void discoverNames();

public:
    Headnode();

    [[nodiscard]] const OS& getOS() const noexcept;

    [[nodiscard]] const std::string& getHostname() const noexcept;
    void setHostname(const std::string& hostname);

    [[nodiscard]] const std::string& getFQDN() const noexcept;
    void setFQDN(const std::string& fqdn);

    //const std::unique_ptr<Connection>& getConnection() const;
    [[nodiscard]] const std::list<Connection>& getConnections() const;
    void addConnection(const Network& network);
    void addConnection(const Network&, const std::string&,
                                       const std::string&);

    //[[nodiscard]] const Connection& getConnection(Network::Profile) const;
    [[nodiscard]] Connection& getConnection(Network::Profile);

};

#endif /* HEADNODE_H */
