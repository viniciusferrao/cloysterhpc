/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/node.h>

Node::Node(std::string_view hostname, OS& os, CPU& cpu,
    std::list<Connection>&& connections, std::optional<BMC> bmc)
    : Server(hostname, os, cpu, std::move(connections), bmc)
{
}

Node::Node() { }

const std::optional<std::string>& Node::getPrefix() const { return m_prefix; }
void Node::setPrefix(const std::optional<std::string>& prefix)
{
    m_prefix = prefix;
}
const std::optional<size_t>& Node::getPadding() const { return m_padding; }
void Node::setPadding(const std::optional<size_t>& padding)
{
    m_padding = padding;
}
const std::optional<boost::asio::ip::address>& Node::getNodeStartIp() const
{
    return m_node_start_ip;
}
void Node::setNodeStartIp(
    const std::optional<boost::asio::ip::address>& nodeStartIp)
{
    m_node_start_ip = nodeStartIp;
}
const std::string& Node::getMACAddress() const { return m_mac_address; }
void Node::setMACAddress(const std::string& macAddress)
{
    m_mac_address = macAddress;
}
const std::optional<std::string>& Node::getNodeRootPassword() const
{
    return m_node_root_password;
}
void Node::setNodeRootPassword(
    const std::optional<std::string>& nodeRootPassword)
{
    m_node_root_password = nodeRootPassword;
}
