/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NODE_H
#define CLOYSTERHPC_NODE_H

#include <list>
#include <string>

#include "connection.h"
#include "network.h"
#include "os.h"
#include "server.h"

class Node : public Server {
public:
    Node(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections,
        std::optional<BMC> bmc = std::nullopt);
};

#endif // CLOYSTERHPC_NODE_H_
