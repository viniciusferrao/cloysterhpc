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
    size_t m_serialPort { 0 };
    size_t m_serialSpeed { 115200 };
    kind m_kind = kind::IPMI;

    BMC(const std::string& address, const std::string& username,
        const std::string& password, int serialPort, int serialSpeed,
        const kind& kind);

    BMC();

    const std::string& getAddress() const;
    void setAddress(const std::string& address);
    const std::string& getUsername() const;
    void setUsername(const std::string& username);
    const std::string& getPassword() const;
    void setPassword(const std::string& password);
    size_t getSerialPort() const;
    void setSerialPort(size_t serialPort);
    size_t getSerialSpeed() const;
    void setSerialSpeed(size_t serialSpeed);
    kind getKind() const;
    void setKind(kind kind);
};

#endif // CLOYSTERHPC_BMC_H_
