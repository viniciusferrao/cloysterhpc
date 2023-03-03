/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell.h"
#include "../functions.h"
#include "log.h"
#include "xcat.h"

#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fmt/format.h>
#include <memory>

#include "../cluster.h"

using cloyster::runCommand;

Shell::Shell(const std::unique_ptr<Cluster>& cluster)
    : m_cluster(cluster)
{
    // Initialize directory tree
    cloyster::createDirectory(installPath);
    cloyster::createDirectory(std::string { installPath } + "/backup");
}

void Shell::disableSELinux()
{
    runCommand("setenforce 0");

    const auto filename = CHROOT "/etc/sysconfig/selinux";

    cloyster::backupFile(filename);
    cloyster::changeValueInConfigurationFile(filename, "SELINUX", "disabled");

    LOG_WARN("SELinux has been disabled");
}

void Shell::configureSELinuxMode()
{
    LOG_INFO("Setting up SELinux");

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
    LOG_INFO("Setting up firewall");

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

        LOG_WARN("Firewalld has been disabled");
    }
}

void Shell::configureFQDN()
{
    LOG_INFO("Setting up hostname");

    runCommand(fmt::format(
        "hostnamectl set-hostname {}", m_cluster->getHeadnode().getFQDN()));
}

// TODO: Proper file parsing
void Shell::configureHostsFile()
{
    LOG_INFO("Setting up additional entries on hosts file");

    auto& headnode = m_cluster->getHeadnode();

    const auto& ip = headnode.getConnection(Network::Profile::External)
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
    LOG_INFO("Setting up timezone");

    runCommand(fmt::format(
        "timedatectl set-timezone {}", m_cluster->getTimezone().getTimezone()));
}

void Shell::configureLocale()
{
    LOG_INFO("Setting up locale");

    runCommand(fmt::format("localectl set-locale {}", m_cluster->getLocale()));
}

void Shell::disableNetworkManagerDNSOverride()
{
    LOG_INFO("Disabling DNS override on NetworkManager");

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

/* This function configure host networks at once with NetworkManager.
 * We enforce that NM is running enabling it with --now and then set default
 * settings and addresses based on data available on the model.
 * At the end of execution we disable DNS override since the headnode machine
 * will be providing the service.
 */
void Shell::configureNetworks(const std::list<Connection>& connections)
{
    LOG_INFO("Setting up networks");

    runCommand("systemctl enable --now NetworkManager");

    for (const auto& connection : std::as_const(connections)) {
        /* For now, we just skip the external network to avoid disconnects */
        if (connection.getNetwork()->getProfile() == Network::Profile::External)
            continue;

        auto interface = connection.getInterface().value();

        std::vector<address> nameservers
            = connection.getNetwork()->getNameservers();
        std::vector<std::string> formattedNameservers;
        for (int i = 0; i <= nameservers.size(); i++) {
            formattedNameservers.emplace_back(nameservers[i].to_string());
        }

        runCommand(fmt::format("nmcli device set {} managed yes", interface));
        runCommand(
            fmt::format("nmcli device set {} autoconnect yes", interface));
        runCommand(
            fmt::format("nmcli connection add con-name {} ifname {} type {} "
                        "mtu {} ipv4.method manual ipv4.address {}/{} "
                        "ipv4.gateway {} ipv4.dns \"{}\" "
                        "ipv4.dns-search {} ipv6.method disabled",
                magic_enum::enum_name(connection.getNetwork()->getProfile()),
                interface,
                magic_enum::enum_name(connection.getNetwork()->getType()),
                connection.getMTU(), connection.getAddress().to_string(),
                connection.getNetwork()->cidr.at(
                    connection.getNetwork()->getSubnetMask().to_string()),
                connection.getNetwork()->getGateway().to_string(),
                fmt::join(formattedNameservers, " "),
                connection.getNetwork()->getDomainName()));
        runCommand(fmt::format("nmcli device connect {}", interface));
    }

    disableNetworkManagerDNSOverride();
}

void Shell::runSystemUpdate()
{
    if (m_cluster->isUpdateSystem()) {
        LOG_INFO("Checking if system updates are available");
        runCommand("dnf -y update");
    }
}

void Shell::installRequiredPackages()
{
    LOG_INFO("Installing required system packages");

    runCommand("dnf -y install wget dnf-plugins-core");
}

void Shell::configureRepositories()
{
    LOG_INFO("Setting up additional repositories");

    runCommand("dnf -y install "
               "https://dl.fedoraproject.org/pub/epel/"
               "epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install "
               "http://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/"
               "ohpc-release-2-1.el8.x86_64.rpm");

    switch (m_cluster->getHeadnode().getOS().getDistro()) {
        case OS::Distro::RHEL:
            runCommand("dnf config-manager --set-enabled "
                       "codeready-builder-for-rhel-8-x86_64-rpms");
            break;

        case OS::Distro::OL:
            runCommand("dnf config-manager --set-enabled "
                       "ol8_codeready_builder");
            break;
    }
}

void Shell::installOpenHPCBase()
{
    LOG_INFO("Installing base OpenHPC packages");

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

void Shell::configureQueueSystem()
{
    LOG_INFO("Setting up the queue system");

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

void Shell::configureInfiniband()
{
    if (const auto& ofed = m_cluster->getOFED()) {
        LOG_INFO("Setting up Infiniband support");
        ofed->install();
    }
}

/* TODO: Restrict by networks */
void Shell::configureNetworkFileSystem()
{
    LOG_INFO("Setting up the Network File System");

    std::string_view filename = CHROOT "/etc/exports";

    cloyster::backupFile(filename);
    cloyster::addStringToFile(filename,
        "/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
        "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\n");

    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Shell::removeMemlockLimits()
{
    LOG_INFO("Removing memlock limits on headnode");

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
    runCommand("dnf -y install openmpi4-gnu9-ohpc mpich-ofi-gnu9-ohpc "
               "mpich-ucx-gnu9-ohpc mvapich2-gnu9-ohpc");

    /* Default OpenHPC environment */
    runCommand("dnf -y install lmod-defaults-gnu9-openmpi4-ohpc");
}

/* This method is the entrypoint of shell based cluster install
 * The first session of the method will configure and install services on the
 * headnode. The last part will do provisioner related settings and image
 * creation for network booting
 */
void Shell::install()
{
    configureSELinuxMode();
    configureFirewall();
    configureFQDN();

    configureHostsFile();
    configureTimezone();
    configureLocale();

    configureNetworks(m_cluster->getHeadnode().getConnections());
    runSystemUpdate();

    // TODO: Pass headnode instead of cluster to reduce complexity
    configureTimeService(m_cluster->getHeadnode().getConnections());

    configureRepositories();
    runSystemUpdate();

    installRequiredPackages();
    installOpenHPCBase();

    configureInfiniband();
    configureNetworkFileSystem();

    configureQueueSystem();
    removeMemlockLimits();

    installDevelopmentComponents();

    const auto& provisionerName { magic_enum::enum_name(
        m_cluster->getProvisioner()) };

    LOG_DEBUG("Setting up the provisioner: {}", provisionerName);
    // std::unique_ptr<Provisioner> provisioner;
    std::unique_ptr<XCAT> provisioner;
    switch (m_cluster->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            provisioner = std::make_unique<XCAT>(m_cluster);
            break;
    }

    LOG_INFO("Setting up compute node images... This may take a while");

    LOG_INFO("[{}] Setting up repositories", provisionerName);
    provisioner->configureRepositories();

    LOG_INFO("[{}] Installing packages", provisionerName);
    provisioner->installPackages();

    LOG_INFO("[{}] Setting up the provisioner", provisionerName);
    provisioner->setup();

    LOG_INFO("[{}] Creating node images", provisionerName);
    provisioner->createImage();

    LOG_INFO("[{}] Adding compute nodes", provisionerName);
    provisioner->addNodes();

    LOG_INFO("[{}] Setting up image on nodes", provisionerName);
    provisioner->setNodesImage();

    LOG_INFO("[{}] Setting up boot settings via IPMI, if available",
        provisionerName);
    provisioner->setNodesBoot();
    provisioner->resetNodes();
}
