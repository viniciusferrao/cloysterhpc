#include <cloysterhpc/services/ansible/roles/network.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {

using namespace cloyster::utils::singleton;

void deleteConnectionIfExists(std::string_view connectionName)
{
    runner()->executeCommand(
        fmt::format("nmcli connection delete \"{}\"", connectionName));
}

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


void configureNetworks(const std::list<Connection>& connections)
{
    LOG_INFO("Setting up networks")

    osservice()->enableService("NetworkManager");

    for (const auto& connection : std::as_const(connections)) {
        /* For now, we just skip the external network to avoid disconnects */
        if (connection.getNetwork()->getProfile() == Network::Profile::External) {
            continue;
        }

        auto interface = connection.getInterface().value();

        std::vector<address> nameservers
            = connection.getNetwork()->getNameservers();
        std::vector<std::string> formattedNameservers;
        for (std::size_t i = 0; i < nameservers.size(); i++) {
            formattedNameservers.emplace_back(nameservers[i].to_string());
        }

        auto opts = options();
        auto connectionName
            = cloyster::utils::enums::toString(connection.getNetwork()->getProfile());
        if (!opts->dryRun

            && runner()->executeCommand(
                   fmt::format("nmcli connection show {}", connectionName))
                == 0) {
            LOG_WARN("Connection exists {}, skipping", connectionName);
            continue;
        }

        deleteConnectionIfExists(connectionName);
        ::runner()->executeCommand(
            fmt::format("nmcli device set {} managed yes", interface));
        ::runner()->executeCommand(
            fmt::format("nmcli device set {} autoconnect yes", interface));
        ::runner()->executeCommand(fmt::format(
            "nmcli connection add con-name {} ifname {} type {} "
            "mtu {} ipv4.method manual ipv4.address {}/{} "
            "ipv4.dns \"{}\" "
            // "ipv4.gateway {} ipv4.dns \"{}\" "
            // @TODO: CFL only do this if we're using xCAT as provisioner
            // @FIXME: This will break Confluent, is it required by xCAT?
            "ipv4.dns-search {} ipv6.method disabled",
            cloyster::utils::enums::toString(
                connection.getNetwork()->getProfile()),
            interface,
            cloyster::utils::enums::toString(
                connection.getNetwork()->getType()),
            connection.getMTU(), connection.getAddress().to_string(),
            connection.getNetwork()->cidr.at(
                connection.getNetwork()->getSubnetMask().to_string()),
            // connection.getNetwork()->getGateway().to_string(),
            fmt::join(formattedNameservers, " "),
            connection.getNetwork()->getDomainName()));

        /* Give network manage some time to settle thing up
         * Avoids: Error: Connection activation failed: IP configuration could
         * not be reserved (no available address, timeout, etc.).
         */
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Breaking my ssh connection during development
        runner()->executeCommand(
            fmt::format("nmcli device connect {}", interface));
    }

    disableNetworkManagerDNSOverride();

}

void configureFQDN()
{
    LOG_INFO("Setting up hostname")

    ::runner()->executeCommand(fmt::format(
        "hostnamectl set-hostname {}", cluster()->getHeadnode().getFQDN()));
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
    const auto connections = cluster()->getHeadnode().getConnections();
    LOG_INFO("Setting up networks")

    configureNetworks(connections);
    configureFQDN();
    configureHostsFile();

}

}
