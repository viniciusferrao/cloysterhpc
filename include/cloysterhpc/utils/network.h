#ifndef CLOYSTERHC_UTILS_NETWORK
#define CLOYSTERHC_UTILS_NETWORK

#include <boost/asio/ip/address.hpp>

namespace cloyster::utils::network {

std::uint8_t subnetMaskToCIDR(const boost::asio::ip::address& addr);

}

#endif
