#include "connection.h"

#include <string>

#include <ifaddrs.h> /* getifaddrs() */

int Connection::setInterfaceName () {
    if (getifaddrs(&this->m_ifaddr) == -1) {
        return -1;
    }
    return 0;
}

auto Connection::getInterfaceName () {
    return (&this->m_ifaddr);
}

/* TODO: Check against /xx declaration on subnetMask */
int Connection::setIPAddress (const std::string& address) {
    if (inet_aton(address.c_str(), &this->m_address) == 0)
        return -1; /* Invalid IP Address */
    return 0;
}

std::string Connection::getIPAddress () {
    return inet_ntoa(m_address);
}
