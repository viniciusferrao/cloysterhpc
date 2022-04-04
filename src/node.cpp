//
// Created by Vinícius Ferrão on 13/11/21.
//
#include "node.h"

Node::Node(OS& os, std::string_view hostname, const Network& network,
           const std::string& address, const std::string& mac,
           std::string_view bmcAddress, std::string_view bmcUsername,
           std::string_view bmcPassword)
           : m_bmcAddress(bmcAddress)
           , m_bmcUsername(bmcUsername)
           , m_bmcPassword(bmcPassword) {

    setOS(os);
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

const std::string& Node::getBMCAddress() const noexcept {
    return m_bmcAddress;
}

void Node::setBMCAddress(const std::string& bmcAddress) {
    m_bmcAddress = bmcAddress;
}

const std::string& Node::getBMCUsername() const noexcept {
    return m_bmcUsername;
}

void Node::setBMCUsername(const std::string& bmcUsername) {
    m_bmcUsername = bmcUsername;
}

const std::string& Node::getBMCPassword() const noexcept {
    return m_bmcPassword;
}

void Node::setBMCPassword(const std::string& bmcPassword) {
    m_bmcPassword = bmcPassword;
}
