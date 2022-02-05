#ifndef CONNECTION_H
#define CONNECTION_H

#include "network.h"

#include <string>
#include <memory>
#include <arpa/inet.h>

/* Each server can have one and only one connection to a given network, although
 * it can have more than one address. This is standard TCP networking.
 */
class Connection {
private:
    const Network& m_network;

    std::string m_interface;
    std::string m_mac;
    // TODO: Use std::vector to support more than one IP address per interface
    //std::vector<struct in_addr> m_address;
    struct in_addr m_address {};

    // TODO: This may not be here
    std::string m_hostname; // Remove
    std::string m_fqdn; // Remove

public:
    Connection() = delete;
    explicit Connection(const Network& network);
    Connection(const Network& network, const std::string&, const std::string&);

    ~Connection();

    // TODO: OOP those methods. There's a lot of code repetition on set/fetch
    [[nodiscard]] std::string getInterface() const;
    void setInterface(const std::string&);
    [[nodiscard]] static std::vector<std::string> fetchInterfaces();

    [[nodiscard]] const std::string &getMAC() const;
    void setMAC(const std::string&);

    [[nodiscard]] std::string getAddress() const;
    void setAddress(const std::string&);
    [[nodiscard]] static std::string fetchAddress(const std::string& interface);

    [[nodiscard]] const std::string& getHostname() const;
    void setHostname(const std::string&);

    [[nodiscard]] const std::string& getFQDN() const;
    void setFQDN(const std::string&);

    [[nodiscard]] const Network& getNetwork() const;
};

#endif /* CONNECTION_H */
