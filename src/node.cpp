/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "node.h"

Node::Node(std::string_view hostname, OS& os, CPU& cpu,
           std::list<Connection>&& connections, std::optional<BMC> bmc)
           : Server(hostname, os, cpu, std::move(connections), bmc) {}
