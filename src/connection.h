#ifndef CONNECTION_H
#define CONNECTION_H

#include "network.h"

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <ifaddrs.h>

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
class Connection {
private:
    //FIXME: Why this cannot be a reference? When creating the network over a
    //       presenter it ended up referencing to an unknown location. It may be
    //       related to lifecycle, since the object has become out of scope, but
    //       we aren't sure.
    const Network m_network;

    std::optional<std::string> m_interface;
    std::optional<std::string> m_mac;
    // TODO: Use std::vector to support more than one IP address per interface
    //std::vector<struct in_addr> m_address;
    struct in_addr m_address {};
    // TODO: MTU is a network parameter
    std::uint16_t m_mtu {1500};

    // TODO: This may not be here
    std::string m_hostname; // Remove
    std::string m_fqdn; // Remove

public:
    Connection() = delete;
    explicit Connection(const Network& network);
    Connection(const Network& network, const std::string& interface,
               const std::string& address);
    Connection(const Network& network,
               std::optional<std::string_view> interface,
               std::optional<std::string_view> mac, const std::string& address);

    Connection(const Connection& other);
    Connection& operator=(const Connection& other) = delete;

    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) = delete;

    ~Connection() = default;

    // TODO: OOP those methods. There's a lot of code repetition on set/fetch
    [[nodiscard]] std::optional<std::string_view> getInterface() const;
    void setInterface(std::string_view interface);
    [[nodiscard]] static std::vector<std::string> fetchInterfaces();

    [[nodiscard]] std::optional<std::string_view> getMAC() const;
    void setMAC(std::string_view mac);

    [[nodiscard]] std::uint16_t getMTU() const;
    void setMTU(std::uint16_t mtu);

    [[nodiscard]] const std::string getAddress() const;
    void setAddress(const std::string&);
    [[nodiscard]] static std::string fetchAddress(const std::string& interface);

    [[nodiscard]] const std::string& getHostname() const;
    void setHostname(const std::string&);

    [[nodiscard]] const std::string& getFQDN() const;
    void setFQDN(const std::string&);

    [[nodiscard]] const Network& getNetwork() const;

#ifndef NDEBUG
    void dumpConnection() const;
#endif

};

#endif /* CONNECTION_H */
