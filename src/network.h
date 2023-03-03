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

#include <magic_enum.hpp>

using boost::asio::ip::address;

/* TODO: Refactoring is necessary
 *  m_domainName is also available here since non-default networks may
 *  exist on the entire cluster and they should have their own domain.
 */
class Network {
public:
    enum class Profile { External, Management, Service, Application };
    enum class Type { Ethernet, Infiniband };

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
    Network(Profile, Type, const address& ip, const address& subnetMask,
        const address& gateway, const uint16_t& vlan,
        const std::string& domainName, const std::vector<address>& nameserver);

    Network(const Network& other) = default;
    Network& operator=(const Network& other) = default;

    Network(Network&& other) = default;
    Network& operator=(Network&& other) = default;

    ~Network() = default;

    // We cannot set or change Profile/Type after instantiation
    [[nodiscard]] const Profile& getProfile() const;
    [[nodiscard]] const Type& getType() const;

    [[nodiscard]] address getAddress() const;
    void setAddress(const address& address);
    [[nodiscard]] static address fetchAddress(const std::string& interface);

    [[nodiscard]] address getSubnetMask() const;
    void setSubnetMask(const address& subnetMask);
    [[nodiscard]] static address fetchSubnetMask(const std::string& interface);

    [[nodiscard]] address getGateway() const;
    void setGateway(const address& gateway);
    [[nodiscard]] static address fetchGateway(const std::string& interface);

    [[nodiscard]] std::uint16_t getVLAN() const;
    void setVLAN(std::uint16_t vlan);

    [[nodiscard]] const std::string& getDomainName() const;
    void setDomainName(const std::string& domainName);
    [[nodiscard]] static std::string fetchDomainName();

    [[nodiscard]] std::vector<address> getNameservers() const;
    void setNameservers(const std::vector<address>& nameservers);
    [[nodiscard]] static std::vector<address> fetchNameservers();

#ifndef NDEBUG
    void dumpNetwork() const;
#endif
};

#endif // CLOYSTERHPC_NETWORK_H_
