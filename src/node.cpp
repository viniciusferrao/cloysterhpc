//
// Created by Vinícius Ferrão on 13/11/21.
//
#include "node.h"

Node::Node(std::string_view hostname, OS& os, CPU& cpu,
           std::list<Connection>&& connections, std::optional<BMC> bmc)
           : Server(hostname, os, cpu, std::move(connections), bmc) {}
