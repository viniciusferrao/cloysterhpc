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
public:
    Node(OS& os, CPU& cpu, std::string_view hostname, const Network& network,
         std::string_view mac, const std::string& address, std::optional<BMC> bmc = std::nullopt);

    Node(OS& os, CPU& cpu, std::string_view hostname,
         std::list<Connection>&& connections, std::optional<BMC> bmc = std::nullopt);

//    // FIXME: We need to rely on the definition on base class
//    [[nodiscard]] const std::list<Connection> &getConnection() const noexcept;
};

#endif // CLOYSTER_NODE_H
