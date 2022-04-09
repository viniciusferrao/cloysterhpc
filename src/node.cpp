//
// Created by Vinícius Ferrão on 13/11/21.
//
#include "node.h"

Node::Node(OS& os, std::optional<BMC> bmc, std::string_view hostname, const Network& network,
           const std::string& address, const std::string& mac) {
    setOS(os);
    m_bmc = bmc;
    setHostname(hostname);
    addConnection(network, address, mac);
}

const std::list<Connection>& Node::getConnection() const noexcept {
    return m_connection;
}

void Node::addConnection(const Network& network,
                         const std::string& address,
                         const std::string& mac) {
    m_connection.emplace_back(network);
    m_connection.back().setMAC(mac);
    m_connection.back().setAddress(address);
}
