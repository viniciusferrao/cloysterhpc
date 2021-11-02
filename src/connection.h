#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <arpa/inet.h>

class Connection {
public:
    std::string hostname;
    std::string fqdn;

private:
    struct ifaddrs *m_ifaddr;
    struct in_addr m_address;

public:
    Connection();
    ~Connection();

    int setInterfaceName();
    auto getInterfaceName();

    int setIPAddress (const std::string&);
    std::string getIPAddress();
};

#endif /* CONNECTION_H */