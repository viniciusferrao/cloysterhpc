/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/services/bmc.h>
BMC::BMC(const std::string& address, const std::string& username,
    const std::string& password, std::size_t serialPort,
    std::size_t serialSpeed, const BMC::kind& kind)
    : m_address(address)
    , m_username(username)
    , m_password(password)
    , m_serialPort(serialPort)
    , m_serialSpeed(serialSpeed)
    , m_kind(kind)
{
}

BMC::BMC() { };

const std::string& BMC::getAddress() const { return m_address; }
void BMC::setAddress(const std::string& address) { m_address = address; }
const std::string& BMC::getUsername() const { return m_username; }
void BMC::setUsername(const std::string& username) { m_username = username; }
const std::string& BMC::getPassword() const { return m_password; }
void BMC::setPassword(const std::string& password) { m_password = password; }
std::size_t BMC::getSerialPort() const { return m_serialPort; }
void BMC::setSerialPort(std::size_t serialPort) { m_serialPort = serialPort; }
std::size_t BMC::getSerialSpeed() const { return m_serialSpeed; }
void BMC::setSerialSpeed(std::size_t serialSpeed)
{
    m_serialSpeed = serialSpeed;
}
BMC::kind BMC::getKind() const { return m_kind; }
void BMC::setKind(BMC::kind kind) { m_kind = kind; }
