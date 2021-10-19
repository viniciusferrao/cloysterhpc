#include "network.h"

#include <ifaddrs.h> /* getifaddrs() */

void Network::setProfile (Network::Profile profile) {
    this->m_profile = profile;
}

Network::Profile Network::getProfile () {
    return this->m_profile;
}

void Network::setType (Network::Type type) {
    this->m_type = type;
}

Network::Type Network::getType () {
    return this->m_type;
}

int Network::setInterfaceName () {
    // if (getifaddrs(&this->m_ifaddr) == -1) {
    //     return -1;
    // }
    m_interfacename = "eth0";
    return 0;
}

void Network::printInterfaceName () {

}

/* TODO: Check against /xx declaration on subnetMask */
int Network::setIPAddress (const std::string& address, const std::string& subnetMask) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        return -1; /* Invalid IP Address */
    if (inet_aton(subnetMask.c_str(), &this->m_subnetmask) == 0)
        return -2; /* Invalid Subnet Mask */
    return 0;
}

std::string Network::getIPAddress () {
    return inet_ntoa(m_address);
}

const std::string &Network::getInterfacename() const {
    return m_interfacename;
}

void Network::setInterfacename(const std::string &interfacename) {
    if (interfacename == "lo") {
        //throw renameThisError;
        return;
    }
    this->m_interfacename = interfacename;
}
