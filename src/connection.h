#ifndef CONNECTION_H
#define CONNECTION_H

#include "network.h"

#include <string>
#include <memory>
#include <arpa/inet.h>
#include <ifaddrs.h>

/* Each server can have one and only one connection to a given network, although
 * it can have more than one address. This is standard TCP networking.
 */
class Connection {
private:
    const Network& m_network;

    std::optional<std::string> m_interface;
    std::string m_mac;
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
               std::string_view mac, const std::string& address);

    // TODO: OOP those methods. There's a lot of code repetition on set/fetch
    [[nodiscard]] std::optional<std::string_view> getInterface() const;
    void setInterface(std::string_view interface);
    [[nodiscard]] static std::vector<std::string> fetchInterfaces();

    [[nodiscard]] std::string_view getMAC() const;
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
};

#endif /* CONNECTION_H */
