/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>

#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <chrono>
#include <fmt/format.h>
#include <memory>

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/queuesystem.h>
#include <cloysterhpc/models/pbs.h>
#include <cloysterhpc/models/slurm.h>

#include <cloysterhpc/dbus_client.h>
#include <ranges>

using cloyster::OS;
using cloyster::runCommand;

namespace {

auto getToEnableRepoNames(const OS& osinfo)
{
    switch (osinfo.getPlatform()) {
        case OS::Platform::el8:
        case OS::Platform::el9:
        case OS::Platform::el10:
            return std::vector<std::string>({ "-beegfs", "-elrepo", "-epel", "-openhpc",
                "-openhpc-updates", "-rpmfusion-free-updates" })
            | std::views::transform([](const std::string& repo) {
                return fmt::format("{}{}", cloyster::productName, repo);
            })
            | std::ranges::to<std::vector<std::string>>();
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

}

namespace cloyster::services {

Shell::Shell(std::unique_ptr<Cluster> cluster)
    : m_cluster(std::move(cluster))
{
    // Initialize directory tree
    cloyster::createDirectory(installPath);
    cloyster::createDirectory(std::string { installPath } + "/backup");
    cloyster::createDirectory(
        std::string { installPath } + "/conf/node/etc/auto.master.d");
}

void Shell::disableSELinux()
{
    runCommand("setenforce 0");

    const auto filename = CHROOT "/etc/sysconfig/selinux";

    cloyster::backupFile(filename);
    cloyster::changeValueInConfigurationFile(filename, "SELINUX", "disabled");

    LOG_WARN("SELinux has been disabled")
}

void Shell::configureSELinuxMode()
{
    LOG_INFO("Setting up SELinux")

    switch (m_cluster->getSELinux()) {
        case Cluster::SELinuxMode::Permissive:
            runCommand("setenforce 0");
            /* Permissive mode */
            break;

        case Cluster::SELinuxMode::Enforcing:
            /* Enforcing mode */
            runCommand("setenforce 1");
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

    if (m_cluster->isFirewall()) {
        runCommand("systemctl enable --now firewalld");

        // Add the management interface as trusted
        runCommand(fmt::format(
            "firewall-cmd --permanent --zone=trusted --change-interface={}",
            m_cluster->getHeadnode()
                .getConnection(Network::Profile::Management)
                .getInterface()
                .value()));

        // If we have IB, also add its interface as trusted
        if (m_cluster->getOFED())
            runCommand(fmt::format(
                "firewall-cmd --permanent --zone=trusted --change-interface={}",
                m_cluster->getHeadnode()
                    .getConnection(Network::Profile::Application)
                    .getInterface()
                    .value()));

        runCommand("firewall-cmd --reload");
    } else {
        runCommand("systemctl disable --now firewalld");

        LOG_WARN("Firewalld has been disabled")
    }
}

void Shell::configureFQDN()
{
    LOG_INFO("Setting up hostname")

    runCommand(fmt::format(
        "hostnamectl set-hostname {}", m_cluster->getHeadnode().getFQDN()));
}

// TODO: Proper file parsing
void Shell::configureHostsFile()
{
    LOG_INFO("Setting up additional entries on hosts file")

    const auto& headnode = m_cluster->getHeadnode();

    const auto& ip = headnode.getConnection(Network::Profile::Management)
                         .getAddress()
                         .to_string();
    const auto& fqdn = headnode.getFQDN();
    const auto& hostname = headnode.getHostname();

    std::string_view filename = CHROOT "/etc/hosts";

    cloyster::backupFile(filename);
    cloyster::addStringToFile(
        filename, fmt::format("{}\t{} {}\n", ip, fqdn, hostname));
}

void Shell::configureTimezone()
{
    LOG_INFO("Setting up timezone")

    runCommand(fmt::format(
        "timedatectl set-timezone {}", m_cluster->getTimezone().getTimezone()));
}

void Shell::configureLocale()
{
    LOG_INFO("Setting up locale")

    runCommand(fmt::format(
        "localectl set-locale {}", m_cluster->getLocale().getLocale()));
}

void Shell::disableNetworkManagerDNSOverride()
{
    LOG_INFO("Disabling DNS override on NetworkManager")

    std::string_view filename
        = CHROOT "/etc/NetworkManager/conf.d/90-dns-none.conf";

    // TODO: We should not violently remove the file, we may need to backup if
    //  the file exists, and remove after the copy
    cloyster::removeFile(filename);
    // TODO: Would be better handled with a .conf function
    cloyster::addStringToFile(filename,
        "[main]\n"
        "dns=none\n");

    runCommand("systemctl restart NetworkManager");
}

// BUG: Why this method exists? The name does not do what it says.
void Shell::deleteConnectionIfExists(std::string_view connectionName)
{
    runCommand(fmt::format("nmcli connection delete \"{}\"", connectionName));
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

    runCommand("systemctl enable --now NetworkManager");

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

        deleteConnectionIfExists(
            magic_enum::enum_name(connection.getNetwork()->getProfile()));
        runCommand(fmt::format("nmcli device set {} managed yes", interface));
        runCommand(
            fmt::format("nmcli device set {} autoconnect yes", interface));
        runCommand(
            fmt::format("nmcli connection add con-name {} ifname {} type {} "
                        "mtu {} ipv4.method manual ipv4.address {}/{} "
                        "ipv4.dns \"{}\" "
                        // "ipv4.gateway {} ipv4.dns \"{}\" "
                        "ipv4.dns-search {} ipv6.method disabled",
                magic_enum::enum_name(connection.getNetwork()->getProfile()),
                interface,
                magic_enum::enum_name(connection.getNetwork()->getType()),
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
        runCommand(fmt::format("nmcli device connect {}", interface));
    }

    disableNetworkManagerDNSOverride();
}

void Shell::runSystemUpdate()
{
    if (m_cluster->isUpdateSystem()) {
        LOG_INFO("Checking if system updates are available")
        runCommand("dnf -y update");
    }
}

void Shell::installRequiredPackages()
{
    LOG_INFO("Installing required system packages")

    runCommand("dnf -y install wget dnf-plugins-core");
}

void Shell::disallowSSHRootPasswordLogin()
{
    LOG_INFO("Allowing root login only through public key authentication (SSH)")

    runCommand(
        "sed -i \"/^#\\?PermitRootLogin/c\\PermitRootLogin without-password\""
        " /etc/ssh/sshd_config");
}

void Shell::installOpenHPCBase()
{
    LOG_INFO("Installing base OpenHPC packages")

    runCommand("dnf -y install ohpc-base");
}

void Shell::configureTimeService(const std::list<Connection>& connections)
{
    LOG_INFO("Setting up time services")

    if (runCommand("rpm -q chrony"))
        runCommand("dnf -y install chrony");

    std::string_view filename = CHROOT "/etc/chrony.conf";

    cloyster::backupFile(filename);

    for (const auto& connection : std::as_const(connections)) {
        if ((connection.getNetwork()->getProfile()
                == Network::Profile::Management)
            || (connection.getNetwork()->getProfile()
                == Network::Profile::Service)) {

            // Configure server as local stratum (serve time without sync)
            cloyster::addStringToFile(filename, "local stratum 10\n");

            cloyster::addStringToFile(filename,
                fmt::format("allow {}/{}\n",
                    connection.getAddress().to_string(),
                    connection.getNetwork()->cidr.at(
                        connection.getNetwork()->getSubnetMask().to_string())));
        }
    }

    runCommand("systemctl enable --now chronyd");
}

using cloyster::models::QueueSystem;
using cloyster::models::SLURM;
using cloyster::models::PBS;

void Shell::configureQueueSystem()
{
    LOG_INFO("Setting up the queue system")

    if (const auto& queue = m_cluster->getQueueSystem()) {
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

                runCommand("dnf -y install openpbs-server-ohpc");
                runCommand("systemctl enable --now pbs");
                runCommand("qmgr -c \"set server default_qsub_arguments= -V\"");
                runCommand(fmt::format(
                    "qmgr -c \"set server resources_default.place={}\"",
                    magic_enum::enum_name<PBS::ExecutionPlace>(
                        pbs->getExecutionPlace())));
                runCommand("qmgr -c \"set server job_history_enable=True\"");
                break;
            }
        }
    }
}

void Shell::configureMailSystem()
{
    LOG_INFO("Setting up the mail system");

    m_cluster->getMailSystem()->setup();
}

void Shell::configureInfiniband()
{
    if (const auto& ofed = m_cluster->getOFED()) {
        LOG_INFO("Setting up Infiniband support")
        ofed->install();
    }
}

void Shell::removeMemlockLimits()
{
    LOG_INFO("Removing memlock limits on headnode")

    std::string_view filename = CHROOT "/etc/security/limits.conf";

    cloyster::backupFile(filename);
    cloyster::addStringToFile(filename,
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

    runCommand("dnf -y install ohpc-autotools hwloc-ohpc spack-ohpc "
               "valgrind-ohpc");

    /* Compiler and MPI stacks */
    runCommand("dnf -y install openmpi4-gnu12-ohpc mpich-ofi-gnu12-ohpc "
               "mpich-ucx-gnu12-ohpc mvapich2-gnu12-ohpc");

    /* Default OpenHPC environment */
    runCommand("dnf -y install lmod-defaults-gnu12-openmpi4-ohpc");
}

void Shell::configureRepositories()
{
    const auto os = m_cluster->getHeadnode().getOS();
    auto repos = cloyster::getRepoManager(os);
    // 1. Install files into /etc, these files are the templates
    //    at include/cloysterhpc/repos/el*/*.repo
    repos->initializeDefaultRepositories();
    // 2. Enable the repositories
    repos->enable(getToEnableRepoNames(os));
    // 3. Commit data to disk
    repos->saveToDisk();
}

/* This method is the entrypoint of shell based cluster install
 * The first session of the method will configure and install services on the
 * headnode. The last part will do provisioner related settings and image
 * creation for network booting
 */
void Shell::install()
{
    auto systemdBus = m_cluster->getDaemonBus();

    configureSELinuxMode();
    configureFirewall();
    configureFQDN();
    disallowSSHRootPasswordLogin();

    configureHostsFile();
    configureTimezone();
    configureLocale();

    configureNetworks(m_cluster->getHeadnode().getConnections());
    runSystemUpdate();

    // TODO: Pass headnode instead of cluster to reduce complexity
    configureTimeService(m_cluster->getHeadnode().getConnections());

    installRequiredPackages();

    configureRepositories();

    runSystemUpdate();

    installOpenHPCBase();

    configureInfiniband();

    // BUG: Broken. Compute nodes does not mount anything.
    NFS networkFileSystem = NFS(systemdBus, "pub", "/opt/ohpc",
        m_cluster->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getAddress(),
        "ro,no_subtree_check");
    networkFileSystem.configure();
    networkFileSystem.enable();
    networkFileSystem.start();

    configureQueueSystem();
    if (m_cluster->getMailSystem().has_value())
        configureMailSystem();
    removeMemlockLimits();

    installDevelopmentComponents();

    const auto& provisionerName { magic_enum::enum_name(
        m_cluster->getProvisioner()) };

    LOG_DEBUG("Setting up the provisioner: {}", provisionerName)
    // std::unique_ptr<Provisioner> provisioner;
    std::unique_ptr<XCAT> provisioner;
    switch (m_cluster->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            provisioner = std::make_unique<XCAT>(m_cluster);
            break;
    }

    LOG_INFO("Setting up compute node images... This may take a while")

    LOG_INFO("[{}] Installing provisioner repositories", provisionerName)
    provisioner->installRepositories();

    LOG_INFO("[{}] Installing provisioner packages", provisionerName)
    provisioner->installPackages();

    LOG_INFO("[{}] Patching the provisioner", provisionerName)
    provisioner->patchInstall();

    LOG_INFO("[{}] Setting up the provisioner", provisionerName)
    provisioner->setup();

    LOG_INFO("[{}] Creating node images", provisionerName)
    provisioner->createImage();

    LOG_INFO("[{}] Adding compute nodes", provisionerName)
    provisioner->addNodes();

    LOG_INFO("[{}] Setting up image on nodes", provisionerName)
    provisioner->setNodesImage();

    LOG_INFO("[{}] Setting up boot settings via IPMI, if available",
        provisionerName);
    provisioner->setNodesBoot();
    provisioner->resetNodes();
}

}
