#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <list>
#include <regex>
#include <fmt/format.h>

#include "os.h"
#include "cpu.h"
#include "services/bmc.h"
#include "connection.h"

class Server {
protected:
    OS m_os;
    //CPU m_cpu;
    std::optional<BMC> m_bmc{};
    std::string m_hostname;
    std::string m_fqdn;
    std::list<Connection> m_connection;

//public:
//    std::string vendor;
//    unsigned processors;
//    unsigned memory;

public:
    [[nodiscard]] const OS& getOS() const noexcept;
    void setOS(const OS& os);

    [[nodiscard]] const std::string& getHostname() const noexcept;
    void setHostname(const std::string& hostname);
    void setHostname(std::string_view hostname);

    [[nodiscard]] const std::string& getFQDN() const noexcept;
    void setFQDN(const std::string& fqdn);

    //const std::unique_ptr<Connection>& getConnection() const;
    [[nodiscard]] const std::list<Connection>& getConnections() const;
    void addConnection(const Network& network);
    virtual void addConnection(const Network& network,
                               const std::string& interface,
                               const std::string& address);
    void addConnection(Connection&& connection);

    //[[nodiscard]] const Connection& getConnection(Network::Profile) const;
    [[nodiscard]] Connection& getConnection(Network::Profile);

    [[nodiscard]] const std::optional<BMC>& getBMC() const;

    virtual ~Server() = default;
};

#endif // SERVER_H
