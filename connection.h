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
    Connection ();

    int setInterfaceName (void);
    auto getInterfaceName (void);

    int setIPAddress (std::string);
    std::string getIPAddress (void);
};

#endif /* CONNECTION_H */