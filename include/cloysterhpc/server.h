/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SERVER_H_
#define CLOYSTERHPC_SERVER_H_

#include <fmt/format.h>
#include <list>
#include <regex>
#include <string>

#include <cloysterhpc/connection.h>
#include <cloysterhpc/cpu.h>
#include <cloysterhpc/os.h>
#include <cloysterhpc/services/bmc.h>

class Server {
protected:
    std::string m_hostname;
    OS m_os;
    CPU m_cpu;
    std::list<Connection> m_connection;
    std::optional<BMC> m_bmc {};
    std::string m_fqdn; // TODO: Remove?

protected:
    Server() = default;
    Server(std::string_view hostname, OS& os, CPU& cpu,
        std::list<Connection>&& connections,
        std::optional<BMC> bmc = std::nullopt);

public:
    [[nodiscard]] const OS& getOS() const noexcept;
    void setOS(const OS& os);

    [[nodiscard]] const std::string& getHostname() const noexcept;
    void setHostname(const std::string& hostname);
    void setHostname(std::string_view hostname);

    // TODO: FQDN should be optional if the domain isn't available
    [[nodiscard]] const std::string& getFQDN() const noexcept;
    void setFQDN(const std::string& fqdn);

    [[nodiscard]] const std::list<Connection>& getConnections() const;
    void addConnection(Network& network);
    void addConnection(Network& network,
        std::optional<std::string_view> interface, std::string_view mac,
        const std::string& ip);
    void addConnection(Connection&& connection);

    [[nodiscard]] const Connection& getConnection(Network::Profile) const;
    //    [[nodiscard]] Connection& getConnection(Network::Profile);

    [[nodiscard]] const CPU& getCPU() const noexcept;
    void setCPU(const CPU& cpu);

    [[nodiscard]] const std::optional<BMC>& getBMC() const;
    void setBMC(const BMC& bmc);

    virtual ~Server() = default;
};

#endif // CLOYSTERHPC_SERVER_H_
