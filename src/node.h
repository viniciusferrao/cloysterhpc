//
// Created by Vinícius Ferrão on 13/11/21.
//
#ifndef CLOYSTER_NODE_H
#define CLOYSTER_NODE_H

#include <string>
#include <list>

#include "server.h"
#include "network.h"
#include "connection.h"
#include "os.h"

class Node : public Server {
public:
    Node(std::string_view hostname, OS& os, CPU& cpu,
         std::list<Connection>&& connections, std::optional<BMC> bmc = std::nullopt);
};

#endif // CLOYSTER_NODE_H
