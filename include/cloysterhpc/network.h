/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NETWORK_H_
#define CLOYSTERHPC_NETWORK_H_

#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include <magic_enum/magic_enum.hpp>

using boost::asio::ip::address;

/* TODO: Refactoring is necessary
 *  m_domainName is also available here since non-default networks may
 *  exist on the entire cluster and they should have their own domain.
 */
/**
 * @class Network
 * @brief A class representing a network configuration.
 *
 * This class provides methods for configuring and retrieving network settings,
 * such as IP addresses, subnet masks, gateways, VLANs, domain names, and name
 * servers.
 */
class Network {
public:
    /**
     * @enum Profile
     * @brief Enum for network profiles.
     */
    enum class Profile { External, Management, Service, Application };

    /**
     * @enum Type
     * @brief Enum for network types.
     */
    enum class Type { Ethernet, Infiniband };

    /**
     * @brief A mapping of subnet masks to their CIDR notation values.
     */
    inline static const std::unordered_map<std::string, uint8_t> cidr
        = { // NOLINT
              { "0.0.0.0", 0 }, { "128.0.0.0", 1 }, { "192.0.0.0", 2 },
              { "224.0.0.0", 3 }, { "240.0.0.0", 4 }, { "248.0.0.0", 5 },
              { "252.0.0.0", 6 }, { "254.0.0.0", 7 }, { "255.0.0.0", 8 },
              { "255.128.0.0", 9 }, { "255.192.0.0", 10 },
              { "255.224.0.0", 11 }, { "255.240.0.0", 12 },
              { "255.248.0.0", 13 }, { "255.252.0.0", 14 },
              { "255.254.0.0", 15 }, { "255.255.0.0", 16 },
              { "255.255.128.0", 17 }, { "255.255.192.0", 18 },
              { "255.255.224.0", 19 }, { "255.255.240.0", 20 },
              { "255.255.248.0", 21 }, { "255.255.252.0", 22 },
              { "255.255.254.0", 23 }, { "255.255.255.0", 24 },
              { "255.255.255.128", 25 }, { "255.255.255.192", 26 },
              { "255.255.255.224", 27 }, { "255.255.255.240", 28 },
              { "255.255.255.248", 29 }, { "255.255.255.252", 30 },
              { "255.255.255.254", 31 }, { "255.255.255.255", 32 }
          };

private:
    Profile m_profile;
    Type m_type;
    address m_address {};
    address m_subnetMask {};
    address m_gateway {};
    uint16_t m_vlan {};
    std::string m_domainName;
    std::vector<address> m_nameservers;

public:
    Network();
    explicit Network(Profile);
    Network(Profile, Type);

    /**
     * @brief Constructs a Network object with the specified parameters.
     *
     * @param profile The network profile.
     * @param type The network type.
     * @param ip The IP address.
     * @param subnetMask The subnet mask.
     * @param gateway The gateway address.
     * @param vlan The VLAN ID.
     * @param domainName The domain name.
     * @param nameserver The list of name servers.
     */
    Network(Profile, Type, const std::string& ip, const std::string& subnetMask,
        const std::string& gateway, const uint16_t& vlan,
        const std::string& domainName, const std::vector<address>& nameserver);

    /**
     * @brief Constructs a Network object with the specified parameters.
     *
     * @param profile The network profile.
     * @param type The network type.
     * @param ip The IP address.
     * @param subnetMask The subnet mask.
     * @param gateway The gateway address.
     * @param vlan The VLAN ID.
     * @param domainName The domain name.
     * @param nameserver The list of name servers.
     */
    Network(Profile, Type, const std::string& ip, const std::string& subnetMask,
        const std::string& gateway, const uint16_t& vlan,
        const std::string& domainName,
        const std::vector<std::string>& nameserver);

    Network(const Network& other) = default;
    Network& operator=(const Network& other) = default;

    Network(Network&& other) = default;
    Network& operator=(Network&& other) = default;

    ~Network() = default;

    // We cannot set or change Profile/Type after instantiation
    [[nodiscard]] const Profile& getProfile() const;
    [[nodiscard]] const Type& getType() const;

    [[nodiscard]] address getAddress() const;
    void setAddress(const address& ip);
    void setAddress(const std::string& ip);

    /**
     * @brief Fetches the address associated with a network interface.
     *
     * @param interface The network interface.
     * @return The address associated with the interface.
     */
    [[nodiscard]] static address fetchAddress(const std::string& interface);

    /**
     * @brief Calculates the network address from a connection address.
     *
     * @param connectionAddress The connection address.
     * @return The calculated network address.
     */
    [[nodiscard]] address calculateAddress(const address& connectionAddress);

    /**
     * @brief Calculates the network address from a connection address.
     *
     * @param connectionAddress The connection address.
     * @return The calculated network address.
     */
    [[nodiscard]] address calculateAddress(
        const std::string& connectionAddress);

    [[nodiscard]] address getSubnetMask() const;
    void setSubnetMask(const address& subnetMask);
    void setSubnetMask(const std::string& subnetMask);

    /**
     * @brief Fetches the subnet mask associated with a network interface.
     *
     * @param interface The network interface.
     * @return The subnet mask associated with the interface.
     */
    [[nodiscard]] static address fetchSubnetMask(const std::string& interface);

    [[nodiscard]] address getGateway() const;
    void setGateway(const address& gateway);
    void setGateway(const std::string& gateway);

    /**
     * @brief Fetches the gateway address associated with a network interface.
     *
     * @param interface The network interface.
     * @return The gateway address associated with the interface.
     */
    [[nodiscard]] static address fetchGateway(const std::string& interface);

    [[nodiscard]] std::uint16_t getVLAN() const;
    void setVLAN(std::uint16_t vlan);

    [[nodiscard]] const std::string& getDomainName() const;
    void setDomainName(const std::string& domainName);

    /**
     * @brief Fetches the domain name of the current network.
     *
     * @return The current network's domain name.
     */
    [[nodiscard]] static std::string fetchDomainName();

    [[nodiscard]] std::vector<address> getNameservers() const;
    void setNameservers(const std::vector<address>& nameservers);
    void setNameservers(const std::vector<std::string>& nameservers);

    /**
     * @brief Fetches the list of name servers of the current network.
     *
     * @return The current network's list of name servers.
     */
    [[nodiscard]] static std::vector<address> fetchNameservers();

#ifndef NDEBUG
    void dumpNetwork() const;
#endif
};

#endif // CLOYSTERHPC_NETWORK_H_
