/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_BMC_H_
#define CLOYSTERHPC_BMC_H_

#include <string>

/**
 * @struct BMC
 * @brief Represents a Baseboard Management Controller (BMC) configuration.
 *
 * This struct encapsulates the necessary information to manage and interact
 * with a BMC, including address, credentials, serial port configuration, and
 * type.
 */
struct BMC {
    /**
     * @enum kind
     * @brief Enumerates the types of BMCs.
     *
     * This enum currently supports only IPMI but can be extended to include
     * other BMC types in the future.
     */
    enum class kind { IPMI };

    std::string m_address;
    std::string m_username;
    std::string m_password;
    std::size_t m_serialPort { 0 };
    std::size_t m_serialSpeed { 115200 };
    kind m_kind = kind::IPMI;

    /**
     * @brief Constructs a BMC object with specified parameters.
     *
     * @param address The IP address or hostname of the BMC.
     * @param username The username for accessing the BMC.
     * @param password The password for accessing the BMC.
     * @param serialPort The serial port number used for communication.
     * @param serialSpeed The speed (baud rate) of the serial port.
     * @param kind The type of BMC.
     */
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
