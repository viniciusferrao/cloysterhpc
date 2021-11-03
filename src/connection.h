#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <arpa/inet.h>

/* Each server can have one and only one connection to a given network, although
 * it can have more than one address. This is standard TCP networking.
 */
class Connection {
private:
    std::string m_interface;
    /* TODO: Use std::vector to support more than one IP address */
    //std::vector<struct in_addr> m_address;
    struct in_addr m_address{};
    std::string m_hostname;
    std::string m_fqdn;

public:
    Connection();
    ~Connection();

    void setInterface(const std::string&);
    const std::string getInterface() const;

    void setAddress(const std::string&);
    const std::string getAddress() const;

    const std::string& getHostname() const;
    void setHostname(const std::string& hostname);

    const std::string& getFQDN() const;
    void setFQDN(const std::string& fqdn);
};

#endif /* CONNECTION_H */