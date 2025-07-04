/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NODE_H
#define CLOYSTERHPC_NODE_H

#include <list>
#include <string>

#include <cloysterhpc/connection.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/models/server.h>
#include <cloysterhpc/network.h>

namespace cloyster::models {
/**
 * @class Node
 * @brief A class representing a node in a server cluster.
 *
 * This class extends the Server class and provides additional attributes and
 * methods specific to a node in the cluster.
 */
class Node : public Server {
public:
    std::optional<std::string> m_prefix;
    std::optional<std::size_t> m_padding;
    std::optional<boost::asio::ip::address> m_node_start_ip;
    std::string m_mac_address;
    std::optional<std::string> m_node_root_password;

    /**
     * @brief Constructs a Node object with the specified parameters.
     *
     * @param hostname The hostname of the node.
     * @param os The operating system installed on the node.
     * @param cpu The CPU configuration of the node.
     * @param connections The network connections of the node.
     * @param bmc Optional parameter representing the Baseboard Management
     * Controller (BMC) of the node.
     */
    Node(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections,
        std::optional<BMC> bmc = std::nullopt);

    Node();

    const std::optional<std::string>& getPrefix() const;
    void setPrefix(const std::optional<std::string>& prefix);
    const std::optional<std::size_t>& getPadding() const;
    void setPadding(const std::optional<std::size_t>& padding);
    const std::optional<boost::asio::ip::address>& getNodeStartIp() const;
    void setNodeStartIp(
        const std::optional<boost::asio::ip::address>& nodeStartIp);
    const std::string& getMACAddress() const;
    void setMACAddress(const std::string& macAddress);
    const std::optional<std::string>& getNodeRootPassword() const;
    void setNodeRootPassword(
        const std::optional<std::string>& nodeRootPassword);
};

}; // namespace cloyster::models {

#endif // CLOYSTERHPC_NODE_H_
