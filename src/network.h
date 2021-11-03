#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <arpa/inet.h>

/* TODO: Refactoring is necessary
 *  Profile and Type must be const, they cannot change after being set.
 *  By default Profile and Type should be External and Ethernet; this must be
 *  done within the constructor.
 *  m_domainName is also available here since non-default networks may
 *  exist on the entire cluster and they should have their own domain.
 */
class Network {
public:
    enum class Profile { External, Management, Service, Application };
    enum class Type { Ethernet, Infiniband };

private:
    const Profile m_profile{};
    const Type m_type{};
    struct in_addr m_address{};
    struct in_addr m_subnetMask{};
    struct in_addr m_gateway{};
    uint16_t m_vlan{};
    std::string m_domainName;
    std::vector<struct in_addr> m_nameserver;

public:
    Network();
    explicit Network(Profile);
    Network(Profile, Type);
    ~Network();

    /* We cannot set or change Profile/Type after instantiation */
    Profile getProfile () const;
    Type getType () const;

    const std::string getAddress() const;
    void setAddress(const std::string&);

    const std::string getSubnetMask() const;
    void setSubnetMask(const std::string&);

    const std::string getGateway() const;
    void setGateway(const std::string&);

    const uint16_t& getVLAN() const;
    void setVLAN(const uint16_t& vlan);

    const std::string& getDomainName() const;
    void setDomainName(const std::string&);

    const std::vector<std::string>& getNameserver() const;
    void setNameserver(const std::vector<std::string>&);
};

#endif /* NETWORK_H */

