#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <utility> /* std::pair */
#include <arpa/inet.h>

class Network {
public:
    enum class Profile { External, Management, Service, Application };
    enum class Type { Ethernet, Infiniband };

    std::string hostname;
    std::string domainname;
    std::string fqdn;
    std::vector<std::string> nameserver;

private:
    Profile m_profile;
    Type m_type;
    //struct ifaddrs *m_ifaddr;
    std::string m_interfacename;
    struct in_addr m_address;
    struct in_addr m_subnetmask;
    struct in_addr m_gateway;
    std::pair<bool, unsigned> vlan;

public:
    void setProfile (Profile);
    Profile getProfile ();

    void setType (Type);
    Type getType ();

    int setInterfaceName ();
    void printInterfaceName ();

    const std::string &getInterfacename() const;
    void setInterfacename(const std::string &interfacename);

    int setIPAddress (const std::string&, const std::string&);
    std::string getIPAddress ();
};

#endif /* NETWORK_H */

