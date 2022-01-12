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
    std::shared_ptr<Network> m_network;

    std::string m_interface;
    std::string m_mac;
    /* TODO: Use std::vector to support more than one IP address */
    //std::vector<struct in_addr> m_address;
    struct in_addr m_address {};

    /* TODO: This may not be here */
    std::string m_hostname; // Remove
    std::string m_fqdn; // Remove

public:
    Connection();
    Connection(const std::string&, const std::string&);
    Connection(std::shared_ptr<Network>,
               const std::string&, const std::string&);

    /* TODO: Remove this constructor */
    Connection(const std::string&, const std::string&, const std::string&,
               const std::string&);
    ~Connection();

    const std::string getInterface() const;
    void setInterface(const std::string&);

    const std::string &getMAC() const;
    void setMAC(const std::string&);

    const std::string getAddress() const;
    void setAddress(const std::string&);

    const std::string& getHostname() const;
    void setHostname(const std::string&);

    const std::string& getFQDN() const;
    void setFQDN(const std::string&);

    const std::shared_ptr<Network>& getNetwork() const;
    void setNetwork(std::shared_ptr<Network>);
};

#endif /* CONNECTION_H */
