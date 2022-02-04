//
// Created by Vinícius Ferrão on 13/11/21.
//
#include "node.h"

Node::Node(std::string_view hostname, const Network& network, const std::string& address,
           const std::string& mac, std::string_view bmcAddress,
           std::string_view bmcUsername, std::string_view bmcPassword)
           : m_hostname(hostname)
           , m_bmcAddress(bmcAddress)
           , m_bmcUsername(bmcUsername)
           , m_bmcPassword(bmcPassword) {

    addConnection(network, address, mac);
}

const std::string& Node::getHostname() const noexcept {
    return m_hostname;
}

void Node::setHostname(const std::string& hostname) {
    m_hostname = hostname;
}

const std::string& Node::getFQDN() const noexcept {
    return m_fqdn;
}

void Node::setFQDN(const std::string& fqdn) {
    m_fqdn = fqdn;
}

const std::list<Connection>& Node::getConnection() const noexcept {
    return m_connection;
}

void Node::addConnection(const Network& network,
                         const std::string& t_address,
                         const std::string& t_mac) {
    m_connection.emplace_back(network);
    m_connection.back().setMAC(t_mac);
    m_connection.back().setAddress(t_address);
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
