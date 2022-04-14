//
// Created by Vinícius Ferrão on 13/11/21.
//
#include "node.h"

Node::Node(OS& os, CPU& cpu, std::string_view hostname, const Network& network,
           std::string_view mac, const std::string& address, std::optional<BMC> bmc) {
    setOS(os);
    setCPU(cpu);
    setHostname(hostname);
    addConnection(network, {}, mac, address);
    m_bmc = bmc;
}

Node::Node(OS& os, CPU& cpu, std::string_view hostname,
           std::list<Connection>&& connections, std::optional<BMC> bmc) {
    setOS(os);
    setCPU(cpu);
    setHostname(hostname);
    m_connection = std::move(connections);
    m_bmc = bmc;
}

//const std::list<Connection>& Node::getConnection() const noexcept {
//    return m_connection;
//}
