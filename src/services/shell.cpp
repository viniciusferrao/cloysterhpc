/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/ansible/roles/aide.h>
#include <cloysterhpc/services/ansible/roles/audit.h>
#include <cloysterhpc/services/ansible/roles/fail2ban.h>
#include <cloysterhpc/services/ansible/roles/spack.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>

#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <chrono>
#include <fmt/format.h>
#include <memory>

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/pbs.h>
#include <cloysterhpc/models/queuesystem.h>
#include <cloysterhpc/models/slurm.h>
#include <cloysterhpc/utils/singleton.h>

#include <cloysterhpc/dbus_client.h>
#include <ranges>

using cloyster::models::Cluster;
using cloyster::models::OS;
using cloyster::services::IOSService;
using cloyster::services::IRunner;

namespace {

void dumpPreInstallState()
{
    using namespace cloyster::services::runner;
    const auto opts = cloyster::utils::singleton::options();

    LOG_INFO("Dumping cluster state before the installation begins")

    LOG_INFO("OS");
    shell::cmd("cat /etc/os-release");

    LOG_INFO("Repositories URLs");
    shell::cmd(
        "grep -EH '^(mirrorlist|baseurl)' /etc/yum.repos.d/*.repo || true");

    LOG_INFO("Packages installed");
    shell::cmd("rpm -qa");

    LOG_INFO("Network configuration");
    shell::cmd("ip a");
    shell::cmd("ip link");

    LOG_INFO("Kernel version");
    shell::cmd("uname -a");

    LOG_INFO("Memory");
    shell::cmd("free -m");

    LOG_INFO("Services running");
    shell::cmd("systemctl --no-pager list-units --plain --type=service --all");

    LOG_INFO("Firewall configuration");
    // firewalld may not be running
    shell::cmd("firewall-cmd --list-all-zones || true");

    LOG_INFO("End of cluster state dump");
    opts->maybeStopAfterStep("dump-cluster-state");
}

auto getToEnableRepoNames(const OS& osinfo)
{
    switch (osinfo.getPlatform()) {
        case OS::Platform::el8:
        case OS::Platform::el9:
        case OS::Platform::el10:
            return std::vector<std::string>({ "beegfs", "elrepo", "epel",
                "openhpc", "openhpcupdates", "rpmfusionfreeupdates" });
        default:
            throw std::logic_error("Not implemented");
    }
}

constexpr auto cluster() { return cloyster::Singleton<Cluster>::get(); }
constexpr auto os() { return cloyster::Singleton<Cluster>::get()->getHeadnode().getOS(); }
constexpr auto runner() { return cloyster::Singleton<IRunner>::get(); }
constexpr auto osservice() { return cloyster::Singleton<IOSService>::get(); }

}

namespace cloyster::services {

Shell::Shell()
{
    // Initialize directory tree
    functions::createDirectory(std::string { installPath } + "/backup");
    functions::createDirectory(
        std::string { installPath } + "/conf/node/etc/auto.master.d");
}

void Shell::disableSELinux()
{
    ::runner()->executeCommand("setenforce 0");

    const auto filename = CHROOT "/etc/sysconfig/selinux";

    functions::backupFile(filename);
    functions::changeValueInConfigurationFile(filename, "SELINUX", "disabled");

    LOG_WARN("SELinux has been disabled")
}

void Shell::configureSELinuxMode()
{
    LOG_INFO("Setting up SELinux")

    switch (cluster()->getSELinux()) {
        case Cluster::SELinuxMode::Permissive:
            ::runner()->executeCommand("setenforce 0");
            /* Permissive mode */
            break;

        case Cluster::SELinuxMode::Enforcing:
            /* Enforcing mode */
            ::runner()->executeCommand("setenforce 1");
            break;

        case Cluster::SELinuxMode::Disabled:
            disableSELinux();
            break;
    }
}

// TODO: Better implementation
//  * Perhaps we could loop through connections and add everything except the
//    external connection as trusted. This way we don't need to check if a given
//    network exists on the cluster.
void Shell::configureFirewall()
{
    LOG_INFO("Setting up firewall")

    if (cluster()->isFirewall()) {
        osservice()->enableService("firewalld");

        // Add the management interface as trusted
        ::runner()->executeCommand(fmt::format(
            "firewall-cmd --permanent --zone=trusted --change-interface={}",
            cluster()
                ->getHeadnode()
                .getConnection(Network::Profile::Management)
                .getInterface()
                .value()));

        // If we have IB, also add its interface as trusted
        if (cluster()->getOFED())
            ::runner()->executeCommand(fmt::format(
                "firewall-cmd --permanent --zone=trusted --change-interface={}",
                cluster()
                    ->getHeadnode()
                    .getConnection(Network::Profile::Application)
                    .getInterface()
                    .value()));

        ::runner()->executeCommand("firewall-cmd --reload");
    } else {
        osservice()->disableService("firewalld");

        LOG_WARN("Firewalld has been disabled")
    }
}

void Shell::configureFQDN()
{
    LOG_INFO("Setting up hostname")

    ::runner()->executeCommand(fmt::format(
        "hostnamectl set-hostname {}", cluster()->getHeadnode().getFQDN()));
}

// TODO: Proper file parsing
void Shell::configureHostsFile()
{
    LOG_INFO("Setting up additional entries on hosts file")

    const auto& headnode = cluster()->getHeadnode();

    const auto& ip = headnode.getConnection(Network::Profile::Management)
                         .getAddress()
                         .to_string();
    const auto& fqdn = headnode.getFQDN();
    const auto& hostname = headnode.getHostname();

    std::string_view filename = CHROOT "/etc/hosts";

    functions::backupFile(filename);
    functions::addStringToFile(
        filename, fmt::format("{}\t{} {}\n", ip, fqdn, hostname));
}

void Shell::configureLocale()
{
    LOG_INFO("Setting up locale")

    ::runner()->executeCommand(
        fmt::format("localectl set-locale {}", cluster()->getLocale()));
}

void Shell::disableNetworkManagerDNSOverride()
{
    LOG_INFO("Disabling DNS override on NetworkManager")

    std::string_view filename
        = CHROOT "/etc/NetworkManager/conf.d/90-dns-none.conf";

    // TODO: We should not violently remove the file, we may need to backup if
    //  the file exists, and remove after the copy
    functions::removeFile(filename);
    // TODO: Would be better handled with a .conf function
    functions::addStringToFile(filename,
        "[main]\n"
        "dns=none\n");

    osservice()->restartService("NetworkManager");
}

// BUG: Why this method exists? The name does not do what it says.
void Shell::deleteConnectionIfExists(std::string_view connectionName)
{
    ::runner()->executeCommand(
        fmt::format("nmcli connection delete \"{}\"", connectionName));
}

/* This function configure host networks at once with NetworkManager.
 * We enforce that NM is running enabling it with --now and then set default
 * settings and addresses based on data available on the model.
 * At the end of execution we disable DNS override since the headnode machine
 * will be providing the service.
 */
void Shell::configureNetworks(const std::list<Connection>& connections)
{
    LOG_INFO("Setting up networks")

    osservice()->enableService("NetworkManager");

    for (const auto& connection : std::as_const(connections)) {
        /* For now, we just skip the external network to avoid disconnects */
        if (connection.getNetwork()->getProfile() == Network::Profile::External)
            continue;

        auto interface = connection.getInterface().value();

        std::vector<address> nameservers
            = connection.getNetwork()->getNameservers();
        std::vector<std::string> formattedNameservers;
        for (std::size_t i = 0; i < nameservers.size(); i++) {
            formattedNameservers.emplace_back(nameservers[i].to_string());
        }

        auto opts = Singleton<Options>::get();
        auto connectionName
            = utils::enums::toString(connection.getNetwork()->getProfile());
        if (!opts->dryRun

            && ::runner()->executeCommand(
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
        ::runner()->executeCommand(
            fmt::format("nmcli device connect {}", interface));
    }

    disableNetworkManagerDNSOverride();
}

void Shell::disallowSSHRootPasswordLogin()
{
    LOG_INFO("Allowing root login only through public key authentication (SSH)")

    ::runner()->executeCommand(
        "sed -i \"/^#\\?PermitRootLogin/c\\PermitRootLogin without-password\""
        " /etc/ssh/sshd_config");
}

void Shell::installOpenHPCBase()
{
    LOG_INFO("Installing base OpenHPC packages")

    osservice()->install("ohpc-base");
}

using cloyster::models::PBS;
using cloyster::models::QueueSystem;
using cloyster::models::SLURM;

void Shell::configureQueueSystem()
{
    LOG_INFO("Setting up the queue system")

    if (const auto& queue = cluster()->getQueueSystem()) {
        switch (queue.value()->getKind()) {
            case QueueSystem::Kind::None: {
                __builtin_unreachable();
            }

            case QueueSystem::Kind::SLURM: {
                const auto& slurm = dynamic_cast<SLURM*>(queue.value().get());
                slurm->installServer();
                slurm->configureServer();
                slurm->enableServer();
                slurm->startServer();
                break;
            }

            case QueueSystem::Kind::PBS: {
                const auto& pbs = dynamic_cast<PBS*>(queue.value().get());

                osservice()->install("openpbs-server-ohpc");
                osservice()->enableService("pbs");
                ::runner()->executeCommand(
                    "qmgr -c \"set server default_qsub_arguments= -V\"");
                ::runner()->executeCommand(fmt::format(
                    "qmgr -c \"set server resources_default.place={}\"",
                    cloyster::utils::enums::toString<PBS::ExecutionPlace>(
                        pbs->getExecutionPlace())));
                ::runner()->executeCommand(
                    "qmgr -c \"set server job_history_enable=True\"");
                break;
            }
        }
    }
}

void Shell::configureMailSystem()
{
    LOG_INFO("Setting up the mail system");

    cluster()->getMailSystem()->setup();
}

void Shell::configureInfiniband()
{
    if (const auto& ofed = cluster()->getOFED()) {
        LOG_INFO("Setting up Infiniband support")
        ofed->install(); // shared pointer
    }
}

void Shell::removeMemlockLimits()
{
    LOG_INFO("Removing memlock limits on headnode")

    std::string_view filename = CHROOT "/etc/security/limits.conf";

    functions::backupFile(filename);
    functions::addStringToFile(filename,
        "* soft memlock unlimited\n"
        "* hard memlock unlimited\n");
}

/* TODO: Third party libraries and some logic to install stacks according to
 *  specifics of the system: Infiniband, PSM, etc.
 */
void Shell::installDevelopmentComponents()
{
    LOG_INFO("Installing OpenHPC tools, development libraries, compilers and "
             "MPI stacks");

    auto opts = cloyster::utils::singleton::options();
    auto ohpcPackages = opts->ohpcPackages;
    osservice()->install(fmt::format("{}", fmt::join(ohpcPackages, " ")));
}

void Shell::configureRepositories()
{
    const auto& osinfo = cluster()->getHeadnode().getOS();
    auto repos = cloyster::Singleton<repos::RepoManager>::get();
    repos->initializeDefaultRepositories();
}

void Shell::pinOSVersion()
{
    if (cloyster::utils::singleton::options()->shouldSkip("pin-os-version")) {
        return;
    }
    LOG_INFO("Pinning OS Version, use `--skip pin-os-version`, to skip");
    // Delegate to IOSService implementation
    cloyster::Singleton<IOSService>::get()->pinOSVersion();
}

/* This method is the entrypoint of shell based cluster install
 * The first session of the method will configure and install services on the
 * headnode. The last part will do provisioner related settings and image
 * creation for network booting
 */
void Shell::install()
{
    namespace ansible = ansible::roles;
    const auto osinfo = os();
    const auto run = [&](ansible::Roles role) {
        ansible::run(role, osinfo);
    };

    run(ansible::Roles::DUMP);
    run(ansible::Roles::CHECK);
    run(ansible::Roles::REPOS);
    run(ansible::Roles::BASE);
    run(ansible::Roles::NETWORK);
    run(ansible::Roles::SSHD);
    run(ansible::Roles::OFED);
    run(ansible::Roles::SELINUX);
    run(ansible::Roles::FIREWALL);
    run(ansible::Roles::LOCALE);
    run(ansible::Roles::TIMESYNC);
    run(ansible::Roles::FAIL2BAN);
    run(ansible::Roles::AUDIT);
    run(ansible::Roles::AIDE);
    run(ansible::Roles::SPACK);
    run(ansible::Roles::NFS);
    run(ansible::Roles::QUEUESYSTEM);
    run(ansible::Roles::OHPC);
    run(ansible::Roles::PROVISIONER);
}

}
