#include <cloysterhpc/utils/containers.h>
#include <cloysterhpc/utils/network.h>

namespace cloyster::utils::network {

std::uint8_t subnetMaskToCIDR(const boost::asio::ip::address& addr)
{
    return containers::associative::findExc(Network::cidr, addr.to_string(),
        "Cannot convert {} to CIDR", addr.to_string());
}

}
