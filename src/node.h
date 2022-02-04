//
// Created by Vinícius Ferrão on 13/11/21.
//
#ifndef CLOYSTER_NODE_H
#define CLOYSTER_NODE_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include <string>
#include <list>

#include "server.h"
#include "network.h"
#include "connection.h"

class Node : public Server {
private:
    // TODO: First three attributes may be moved to Server class
    std::string m_hostname;
    std::string m_fqdn;
    std::list<Connection> m_connection;
    std::string m_bmcAddress;
    std::string m_bmcUsername;
    std::string m_bmcPassword;

public:
    Node(std::string_view, const Network& network, const std::string&, const std::string&,
         std::string_view, std::string_view, std::string_view);

    [[nodiscard]] const std::string &getHostname() const noexcept;
    void setHostname(const std::string&);

    [[nodiscard]] const std::string &getFQDN() const noexcept;
    void setFQDN(const std::string&);

    [[nodiscard]] const std::list<Connection> &getConnection() const noexcept;
    void addConnection(const Network& network, const std::string&, const std::string&);

    const std::string& getBMCAddress() const noexcept;
    void setBMCAddress(const std::string& bmcAddress);

    const std::string& getBMCUsername() const noexcept;
    void setBMCUsername(const std::string& bmcUsername);

    const std::string& getBMCPassword() const noexcept;
    void setBMCPassword(const std::string& bmcPassword);
};

#endif // CLOYSTER_NODE_H
