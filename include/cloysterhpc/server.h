/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SERVER_H_
#define CLOYSTERHPC_SERVER_H_

#include <expected>
#include <fmt/format.h>
#include <list>
#include <regex>
#include <string>

#include <cloysterhpc/connection.h>
#include <cloysterhpc/cpu.h>
#include <cloysterhpc/os.h>
#include <cloysterhpc/services/bmc.h>

/**
 * @class Server
 * @brief Represents a server in a cluster.
 *
 * This class encapsulates the properties and behavior of a server in a cluster,
 * including its hostname, operating system, CPU configuration, network
 * connections, and Baseboard Management Controller (BMC).
 */
class Server {
protected:
    std::string m_hostname;
    OS m_os;
    CPU m_cpu;
    std::list<Connection> m_connection;
    std::optional<BMC> m_bmc {};
    std::string m_fqdn; // TODO: Remove?

protected:
    /**
     * @brief Protected constructor to initialize a Server object.
     * @param hostname The hostname of the server.
     * @param os The operating system installed on the server.
     * @param cpu The CPU configuration of the server.
     * @param connections The network connections of the server.
     * @param bmc Optional Baseboard Management Controller (BMC) of the server.
     */
    Server(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections,
        std::optional<BMC> bmc = std::nullopt);

public:
    [[nodiscard]] const OS& getOS() const noexcept;
    void setOS(const OS& os);

    [[nodiscard]] const std::string& getHostname() const noexcept;
    void setHostname(const std::string& hostname);
    void setHostname(std::string_view hostname);

    /**
     * @brief Validate the hostname
     *
     * Return the hostname if is valid, or an error message
     */
    std::expected<std::string_view, std::string> validateHostname();

    // TODO: FQDN should be optional if the domain isn't available
    [[nodiscard]] const std::string& getFQDN() const noexcept;
    void setFQDN(const std::string& fqdn);

    std::expected<std::string_view, std::string> validateFQDN();

    [[nodiscard]] const std::list<Connection>& getConnections() const;

    /**
     * @brief Adds a network connection to the server.
     * @param network The network to add.
     */
    void addConnection(Network& network);

    /**
     * @brief Adds a network connection to the server.
     * @param network The network to add.
     * @param interface The interface for the connection (optional).
     * @param mac The MAC address for the connection.
     * @param ip The IP address for the connection.
     */
    void addConnection(Network& network,
        std::optional<std::string_view> interface, std::string_view mac,
        const std::string& ip);

    /**
     * @brief Adds a network connection to the server.
     * @param connection The connection to add.
     */
    void addConnection(Connection&& connection);

    void setConnection(const std::list<Connection>& connection);

    [[nodiscard]] const Connection& getConnection(Network::Profile) const;
    //    [[nodiscard]] Connection& getConnection(Network::Profile);

    [[nodiscard]] const CPU& getCPU() const noexcept;
    void setCPU(const CPU& cpu);

    [[nodiscard]] const std::optional<BMC>& getBMC() const;
    void setBMC(const BMC& bmc);

    virtual ~Server() = default;
    Server() = default;
};

#endif // CLOYSTERHPC_SERVER_H_
