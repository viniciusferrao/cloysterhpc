/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_BMC_H_
#define CLOYSTERHPC_BMC_H_

#include <string>

struct BMC {
    enum class kind { IPMI };

    std::string m_address;
    std::string m_username;
    std::string m_password;
    std::size_t m_serialPort { 0 };
    std::size_t m_serialSpeed { 115200 };
    kind m_kind = kind::IPMI;

    BMC(const std::string& address, const std::string& username,
        const std::string& password, std::size_t serialPort,
        std::size_t serialSpeed, const kind& kind);

    BMC();

    const std::string& getAddress() const;
    void setAddress(const std::string& address);
    const std::string& getUsername() const;
    void setUsername(const std::string& username);
    const std::string& getPassword() const;
    void setPassword(const std::string& password);
    std::size_t getSerialPort() const;
    void setSerialPort(std::size_t serialPort);
    std::size_t getSerialSpeed() const;
    void setSerialSpeed(std::size_t serialSpeed);
    kind getKind() const;
    void setKind(kind kind);
};

#endif // CLOYSTERHPC_BMC_H_
