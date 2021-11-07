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
    struct in_addr m_address {};

    /* TODO: This may not be here */
    std::string m_hostname; // Remove
    std::string m_fqdn; // Remove

public:
    Connection();
    Connection(const std::string&, const std::string&);
    Connection(const std::string&, const std::string&, const std::string&,
               const std::string&);
    ~Connection();

    const std::string getInterface() const;
    void setInterface(const std::string&);

    const std::string getAddress() const;
    void setAddress(const std::string&);

    const std::string& getHostname() const;
    void setHostname(const std::string&);

    const std::string& getFQDN() const;
    void setFQDN(const std::string&);
};

#endif /* CONNECTION_H */
