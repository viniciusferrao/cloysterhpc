/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CONNECTION_H_
#define CLOYSTERHPC_CONNECTION_H_

#include <cloysterhpc/network.h>

#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <ifaddrs.h>
#include <memory>
#include <string>
#include <expected>

#include <gsl/gsl-lite.hpp>

/* Each server can have one and only one connection to a given network, although
 * it can have more than one address on the same interface. This may seem
 * incorrect, but it is standard TCP networking.
 *
 * Therefore, every connection must have a const ref to a given network.
 * Interface names and MAC addresses are always optional since we may be dealing
 * with an DHCP network we aren't required to know which interface name exists
 * on the other side. On other hand MAC addresses may be simply unavailable if
 * a given Connection is from Infiniband for example, and we also don't need to
 * know MAC addresses in advance if the interface will be configured with static
 * IP addresses
 */

using boost::asio::ip::address;

/**
 * @class Connection
 * @brief Manages network connection details for a cluster node.
 *
 * The Connection class provides functionality to manage the network details
 * for a cluster node, including interface names, MAC addresses, IP addresses,
 * MTU settings, and more.
 */
class Connection {
private:
    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c12-dont-make-data-members-const-or-references
    gsl::not_null<Network*> m_network;

    std::optional<std::string> m_interface;
    std::optional<std::string> m_mac;
    // TODO: Use std::vector to support more than one IP address per interface
    // std::vector<struct in_addr> m_address;
    address m_address {};
    // TODO: MTU is a network parameter
    std::uint16_t m_mtu { 1500 };

    // TODO: This may not be here
    std::string m_hostname; // Remove
    std::string m_fqdn; // Remove

public:
    Connection() = delete;
    explicit Connection(Network* network);

    /**
     * @brief Constructs a Connection object with the given parameters.
     *
     * @param network Pointer to the Network object.
     * @param interface Optional network interface name.
     * @param mac Optional MAC address.
     * @param ip IP address as a string.
     */
    Connection(Network* network, std::optional<std::string_view> interface,
        std::optional<std::string_view> mac, const std::string& ip);

    //    Connection(const Connection& other) = default;
    //    Connection& operator=(const Connection& other) = delete;
    //
    //    Connection(Connection&& other) = default;
    //    Connection& operator=(Connection&& other) = delete;

    ~Connection() = default;

    // TODO: OOP those methods. There's a lot of code repetition on set/fetch
    [[nodiscard]] std::optional<std::string_view> getInterface() const;
    void setInterface(std::string_view interface);

    /**
     * @brief Fetches the list of network interfaces.
     *
     * @return A vector of network interface names.
     */
    [[nodiscard]] static std::vector<std::string> fetchInterfaces();

    [[nodiscard]] std::optional<std::string_view> getMAC() const;
    void setMAC(std::string_view mac);

    static std::expected<bool, std::string> validateMAC(std::string_view address);
    
    [[nodiscard]] std::uint16_t getMTU() const;
    void setMTU(std::uint16_t mtu);

    [[nodiscard]] const address getAddress() const;
    void setAddress(const address& address);
    void setAddress(const std::string& address);

    /**
     * @brief Increments the IP address by the given amount.
     *
     * @param increment Amount to increment the IP address by.
     */
    void incrementAddress(const std::size_t increment = 1) noexcept;

    /**
     * @brief Fetches the IP address of a given network interface.
     *
     * @param interface Network interface name.
     * @return IP address.
     */
    [[nodiscard]] static address fetchAddress(const std::string& interface);

    [[nodiscard]] const std::string& getHostname() const;
    void setHostname(const std::string& hostname);

    [[nodiscard]] const std::string& getFQDN() const;
    void setFQDN(const std::string& fqdn);

    [[nodiscard]] const Network* getNetwork() const;

#ifndef NDEBUG
    /**
     * @brief Dumps the connection details for debugging purposes.
     */
    void dumpConnection() const;
#endif
};

#endif // CLOYSTERHPC_CONNECTION_H_
