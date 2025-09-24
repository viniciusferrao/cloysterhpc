#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/ansible/roles/network.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/utils/network.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {

using namespace cloyster::utils::singleton;
using namespace cloyster::services::runner;

void disableNetworkManagerDNSOverride()
{
    LOG_INFO("Disabling DNS override on NetworkManager")

    std::string_view filename
        = CHROOT "/etc/NetworkManager/conf.d/90-dns-none.conf";

    // TODO: We should not violently remove the file, we may need to backup if
    //  the file exists, and remove after the copy
    cloyster::functions::removeFile(filename);
    // TODO: Would be better handled with a .conf function
    cloyster::functions::addStringToFile(filename,
        "[main]\n"
        "dns=none\n");

    osservice()->restartService("NetworkManager");
}

// WARNING: We used to do this in a DRY way, but each connection has its own
// idissiocracies. Keep connection setup splitted from now on

void configureManagementNetwork(const Connection& connection)
{
    const std::string_view ipv6Method = cluster()->getProvisioner()
            == cloyster::models::Cluster::Provisioner::xCAT
        ? "disabled"
        : "link-local";

    auto interface = connection.getInterface().value();
    auto connectionName = cloyster::utils::enums::toString(
        connection.getNetwork()->getProfile());
    LOG_INFO("Setting up {} network", connectionName);
    LOG_ASSERT(connectionName == "Management",
        "configureManagementNetwork called with invalid network")

    shell::fmt(R"(
nmcli device set {iface} managed yes
nmcli device set {iface} autoconnect yes

# Remove existing connection if present
nmcli con show {iface} > /dev/null && nmcli conn delete {iface}
nmcli con show {conn_name} > /dev/null && nmcli connection delete {conn_name}

# Add new static IPv4 + link-local IPv6 connection
nmcli connection add type {type} mtu {mtu} ifname {iface} con-name {conn_name} \
    ipv4.method manual \
    ipv4.addresses "{ip}/{cidr}" \
    ipv4.gateway "" \
    ipv4.ignore-auto-dns yes \
    ipv4.ignore-auto-routes yes \
    ipv6.method {ipv6_method}
sleep 0.2
nmcli device connect {iface}
)",
        fmt::arg("iface", interface), fmt::arg("conn_name", connectionName),
        fmt::arg("type", connection.getNetwork()->getType()),
        fmt::arg("mtu", connection.getMTU()),
        fmt::arg("ip", connection.getAddress().to_string()),
        fmt::arg("cidr",
            cloyster::utils::network::subnetMaskToCIDR(
                connection.getNetwork()->getSubnetMask())),
        fmt::arg("ipv6_method", ipv6Method));
}

void configureApplicationNetwork(const Connection& connection)
{
    auto interface = connection.getInterface().value();
    auto connectionName = cloyster::utils::enums::toString(
        connection.getNetwork()->getProfile());
    LOG_INFO("Setting up {} network", connectionName);
    LOG_ASSERT(connectionName == "Application",
        "configureApplicationNetwork called with invalid network")

    shell::fmt(R"(
nmcli device set {iface} managed yes
nmcli device set {iface} autoconnect yes

# Remove existing connection if present
nmcli con show {iface} > /dev/null && nmcli conn delete {iface}
nmcli con show {conn_name} > /dev/null && nmcli connection delete {conn_name}

# Add new static IPv4 + link-local IPv6 connection
nmcli connection add type {type} mtu {mtu} ifname {iface} con-name {conn_name} \
    ipv4.method manual \
    ipv4.addresses "{ip}/{cidr}" \
    ipv4.gateway "" \
    ipv4.ignore-auto-dns yes \
    ipv4.ignore-auto-routes yes \
sleep 0.2
nmcli device connect {iface}
)",
        fmt::arg("iface", interface), fmt::arg("conn_name", connectionName),
        fmt::arg("type", connection.getNetwork()->getType()),
        fmt::arg("mtu", connection.getMTU()),
        fmt::arg("ip", connection.getAddress().to_string()),
        fmt::arg("cidr",
            cloyster::utils::network::subnetMaskToCIDR(
                connection.getNetwork()->getSubnetMask())));
}

void configureServiceNetwork(const Connection& connection)
{
    auto interface = connection.getInterface().value();
    auto connectionName = cloyster::utils::enums::toString(
        connection.getNetwork()->getProfile());
    LOG_INFO("Setting up {} network", connectionName);
    LOG_ASSERT(connectionName == "Service",
        "configureServiceNetwork called with invalid network")

    shell::fmt(R"(
nmcli device set {iface} managed yes
nmcli device set {iface} autoconnect yes

# Remove existing connection if present
nmcli con show {iface} > /dev/null && nmcli conn delete {iface}
nmcli con show {conn_name} > /dev/null && nmcli connection delete {conn_name}

# Add new static IPv4 + link-local IPv6 connection
nmcli connection add type {type} mtu {mtu} ifname {iface} con-name {conn_name} \
    ipv4.method manual \
    ipv4.addresses "{ip}/{cidr}" \
    ipv4.gateway "" \
    ipv4.ignore-auto-dns yes \
    ipv4.ignore-auto-routes yes \
sleep 0.2
nmcli device connect {iface}
)",
        fmt::arg("iface", interface), fmt::arg("conn_name", connectionName),
        fmt::arg("type", connection.getNetwork()->getType()),
        fmt::arg("mtu", connection.getMTU()),
        fmt::arg("ip", connection.getAddress().to_string()),
        fmt::arg("cidr",
            cloyster::utils::network::subnetMaskToCIDR(
                connection.getNetwork()->getSubnetMask())));
}

void configureNetworks(const std::list<Connection>& connections)
{
    osservice()->enableService("NetworkManager");
    disableNetworkManagerDNSOverride();

    for (const auto& connection : std::as_const(connections)) {
        if (!connection.getInterface().has_value()) {
            LOG_WARN("Interface not found for connection {}, skipping",
                connection.getNetwork()->getProfile());
            continue;
        }

        switch (connection.getNetwork()->getProfile()) {
            case Network::Profile::External:
                continue;
            case Network::Profile::Management:
                configureManagementNetwork(connection);
                break;
            case Network::Profile::Application:
                configureApplicationNetwork(connection);
                break;
            case Network::Profile::Service:
                configureServiceNetwork(connection);
                break;
            default:
                // NOTE: This should never happen
                cloyster::functions::abort("Invalid network profile {}",
                    connection.getNetwork()->getProfile());
        }
        break;
    }
}

void configureFQDN()
{
    LOG_INFO("Setting up hostname")

    shell::fmt(
        "hostnamectl set-hostname {}", cluster()->getHeadnode().getFQDN());
}

void configureHostsFile()
{
    LOG_INFO("Setting up additional entries on hosts file")

    const auto& headnode = cluster()->getHeadnode();

    const auto& ip = headnode.getConnection(Network::Profile::Management)
                         .getAddress()
                         .to_string();
    const auto& fqdn = headnode.getFQDN();
    const auto& hostname = headnode.getHostname();

    std::string_view filename = CHROOT "/etc/hosts";

    cloyster::functions::backupFile(filename);
    cloyster::functions::addStringToFile(
        filename, fmt::format("{}\t{} {}\n", ip, fqdn, hostname));
}

}

namespace cloyster::services::ansible::roles::network {

void run(const Role& role)
{
    LOG_INFO("Setting up networks, use `--skip network` to skip")
    if (utils::singleton::options()->shouldSkip("network")) {
        return;
    }

    const auto connections = cluster()->getHeadnode().getConnections();
    configureNetworks(connections);
    configureFQDN();
    configureHostsFile();
}

}
