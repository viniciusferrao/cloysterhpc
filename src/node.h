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
#include "os.h"

class Node : public Server {
private:
    std::string m_bmcAddress;
    std::string m_bmcUsername;
    std::string m_bmcPassword;

public:
    Node(OS& os, std::string_view hostname, const Network& network,
         const std::string& address, const std::string& mac,
         std::string_view bmcAddress, std::string_view bmcUsername,
         std::string_view bmcPassword);

    // FIXME: We need to rely on the definition on base class
    [[nodiscard]] const std::list<Connection> &getConnection() const noexcept;
    void addConnection(const Network& network, const std::string& address,
                       const std::string& mac) override;

    [[nodiscard]] const std::string& getBMCAddress() const noexcept;
    void setBMCAddress(const std::string& bmcAddress);

    [[nodiscard]] const std::string& getBMCUsername() const noexcept;
    void setBMCUsername(const std::string& bmcUsername);

    [[nodiscard]] const std::string& getBMCPassword() const noexcept;
    void setBMCPassword(const std::string& bmcPassword);
};

#endif // CLOYSTER_NODE_H
