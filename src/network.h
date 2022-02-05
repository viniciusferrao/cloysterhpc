#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <vector>
#include <arpa/inet.h>
#include <unordered_map>

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
    const std::unordered_map<Profile, std::string> getProfileString = {
        {Profile::External,    "External"},
        {Profile::Management,  "Management"},
        {Profile::Service,     "Service"},
        {Profile::Application, "Application" }
    };

    enum class Type { Ethernet, Infiniband };
    const std::unordered_map<Type, std::string> getTypeString = {
        {Type::Ethernet,   "Ethernet"},
        {Type::Infiniband, "Infiniband"}
    };

    const std::unordered_map<std::string, uint8_t> cidr = {
        {"0.0.0.0",          0},
        {"128.0.0.0",        1},
        {"192.0.0.0",        2},
        {"224.0.0.0",        3},
        {"240.0.0.0",        4},
        {"248.0.0.0",        5},
        {"252.0.0.0",        6},
        {"254.0.0.0",        7},
        {"255.0.0.0",        8},
        {"255.128.0.0",      9},
        {"255.192.0.0",     10},
        {"255.224.0.0",     11},
        {"255.240.0.0",     12},
        {"255.248.0.0",     13},
        {"255.252.0.0",     14},
        {"255.254.0.0",     15},
        {"255.255.0.0",     16},
        {"255.255.128.0",   17},
        {"255.255.192.0",   18},
        {"255.255.224.0",   19},
        {"255.255.240.0",   20},
        {"255.255.248.0",   21},
        {"255.255.252.0",   22},
        {"255.255.254.0",   23},
        {"255.255.255.0",   24},
        {"255.255.255.128", 25},
        {"255.255.255.192", 26},
        {"255.255.255.224", 27},
        {"255.255.255.240", 28},
        {"255.255.255.248", 29},
        {"255.255.255.252", 30},
        {"255.255.255.254", 31},
        {"255.255.255.255", 32}
    };

private:
    const Profile m_profile;
    const Type m_type;
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
    Network(Profile, Type, const std::string& address, const std::string& subnetMask,
            const std::string& gateway, const uint16_t& vlan, const std::string& domainName,
            const std::vector<std::string>& nameserver);
    ~Network();

    // We cannot set or change Profile/Type after instantiation
    [[nodiscard]] const Profile& getProfile () const;
    [[nodiscard]] const Type& getType () const;

    const std::string getAddress() const;
    void setAddress(const std::string&);
    [[nodiscard]] static std::string fetchAddress(const std::string& interface);

    const std::string getSubnetMask() const;
    void setSubnetMask(const std::string&);
    [[nodiscard]] static std::string fetchSubnetMask(const std::string& interface);

    const std::string getGateway() const;
    void setGateway(const std::string&);
    [[nodiscard]] static std::string fetchGateway(const std::string& interface);

    const uint16_t& getVLAN() const;
    void setVLAN(const uint16_t& vlan);

    const std::string& getDomainName() const;
    void setDomainName(const std::string&);

    std::vector<std::string> getNameserver() const;
    void setNameserver(const std::vector<std::string>&);

#ifndef _NDEBUG_
    void dumpNetwork() const;
#endif
};

#endif /* NETWORK_H */

