//
// Created by Vinícius Ferrão on 31/10/21.
//
#include "shell.h"
#include "xcat.h"
#include "../functions.h"

#ifdef _DEBUG_
#include <iostream>
#endif
#include <memory>
#include <fmt/format.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/process.hpp>

#include "../cluster.h"

int Shell::runCommand(const std::string& command) {
#ifndef _DUMMY_
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    m_commandOutput.erase();
    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty()) {
#ifdef _DEBUG_
        std::cerr << line << std::endl;
#endif
        m_commandOutput += line;
        c.wait();
    }

    return c.exit_code();

#else
    std::cout << "exec: " << command << std::endl;
    return 0;
#endif
}

void Shell::disableSELinux () {
    runCommand("setenforce 0");

#ifdef _DUMMY_
    const std::string filename = "chroot/etc/sysconfig/selinux";
#else
    const std::string filename = "/etc/sysconfig/selinux";
#endif

    cloyster::backupFile(filename);
    cloyster::changeValueInConfigurationFile(filename, "SELINUX", "disabled");
}

void Shell::configureSELinuxMode (Cluster::SELinuxMode mode) {
    switch (mode) {
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

        default:
            throw; /* Invalid mode */
    }
}

/* TODO: Better implementation */
void Shell::configureFirewall(bool enabled) {
    if (enabled)
        runCommand("systemctl enable --now firewalld");
    else
        runCommand("systemctl disable --now firewalld");
}

void Shell::configureFQDN (const std::string& fqdn) {
    runCommand(fmt::format("hostnamectl set hostname {}", fqdn));
}

/* TODO: Proper file parsing */
void Shell::configureHostsFile (std::string_view ip, std::string_view fqdn,
                                std::string_view hostname) {

#ifdef _DUMMY_
    std::string_view filename = "chroot/etc/hosts";
#else
    std::string_view filename = "/etc/hosts";
#endif

    cloyster::addStringToFile(filename,
                            fmt::format("{}\t{} {}\n", ip, fqdn, hostname));
}

void Shell::configureTimezone (const std::string& timezone) {
    runCommand(fmt::format("timedatectl set timezone {}", timezone));
}

void Shell::configureLocale (const std::string& locale) {
    runCommand(fmt::format("localectl set locale {}", locale));
}

void Shell::disableNetworkManagerDNSOverride () {
#ifdef _DUMMY_
    std::string_view filename =
            "chroot/etc/NetworkManager/conf.d/90-dns-none.conf";
#else
    std::string_view filename = "/etc/NetworkManager/conf.d/90-dns-none.conf";
#endif

    /* TODO: Would be better handled with a .conf function */
    cloyster::addStringToFile(filename, "[main]\n"
                                        "dns=none\n");

    runCommand("systemctl restart NetworkManager");
}

/* This function configure host networks at once with NetworkManager.
 * We enforce that NM is running enabling it with --now and then set default
 * settings and addresses based on data available on the model.
 * At the end of execution we disable DNS override since the headnode machine
 * will be providing the service.
 * TODO: Get profile and type as string
 */
void Shell::configureNetworks(const std::unique_ptr<Cluster>& cluster) {
    runCommand("systemctl enable --now NetworkManager");

    const std::vector<Connection> connections =
            cluster->getHeadnode().getConnections();

    for (auto const& connection : std::as_const(connections)) {
        /* For now, we just skip the external network to avoid disconnects */
        if (connection.getNetwork()->getProfile() == Network::Profile::External)
            continue;

        auto interface = connection.getInterface();

        runCommand(fmt::format(
                "nmcli device set {} managed yes", interface));
        runCommand(fmt::format(
                "nmcli device set {} autoconnect yes", interface));
        runCommand(fmt::format(
                "nmcli device connect {}", interface));
        runCommand(fmt::format(
                "nmcli connection add con-name {} ifname {} type {} "
                "mtu 1500 ipv4.method manual ipv4.address {}/{} "
                "ipv4.gateway {} ipv4.dns \"{}\" "
                "ipv4.dns-search {} ipv6.method disabled",
                connection.getNetwork()->getProfile(),
                interface,
                connection.getNetwork()->getType(),
                connection.getAddress(),
                connection.getNetwork()->cidr.at(
                        connection.getNetwork()->getSubnetMask()),
                connection.getNetwork()->getGateway(),
                fmt::join(connection.getNetwork()->getNameserver(), " "),
                connection.getNetwork()->getDomainName()));
    }

    disableNetworkManagerDNSOverride();
}

void Shell::runSystemUpdate (bool run) {
    if (run)
        runCommand("dnf -y update");
}

void Shell::installRequiredPackages () {
    runCommand("dnf -y install wget dnf-plugins-core");
}

void Shell::configureRepositories (const std::unique_ptr<Cluster>& cluster) {
    runCommand("dnf -y install "
               "https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install "
               "https://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm");

    switch (cluster->getHeadnode().getOS().getDistro()) {
        case OS::Distro::RHEL:
            runCommand("dnf config-manager --set-enabled "
                       "codeready-builder-for-rhel-8-x86_64-rpms");
            break;

        case OS::Distro::OL:
            runCommand("dnf config-manager --set-enabled "
                       "ol8_codeready_builder");
            break;

        default:
            throw; /* Unsupported OS */
    }
}

void Shell::installOpenHPCBase () {
    runCommand("dnf -y install ohpc-base");
}

void Shell::configureTimeService () {
    if (runCommand("rpm -q chrony"))
        runCommand("dnf -y install chrony");

#ifdef _DUMMY_
    std::string_view filename = "chroot/etc/chrony.conf";
#else
    std::string_view filename = "/etc/chrony.conf";
#endif

    /* TODO: Restrict by networks */
    cloyster::addStringToFile(filename, "allow all");

    runCommand("systemctl enable --now chronyd");
}

void Shell::configureQueueSystem (const std::unique_ptr<Cluster>& cluster) {
    // if (Cluster::QueueSystem::SLURM)
    runCommand("dnf -y install ohpc-slurm-server");
    runCommand("cp /etc/slurm/slurm.conf.ohpc /etc/slurm/slurm.conf");
    runCommand(fmt::format("perl -pi -e "
                           "\"s/ControlMachine=\\S+/ControlMachine={}/\" "
                           "/etc/slurm/slurm.conf",
                           cluster->getHeadnode().getFQDN()));
    runCommand("systemctl enable --now munge");
    runCommand("systemctl enable --now slurmctld");
}

void Shell::configureInfiniband (const std::unique_ptr<Cluster>& cluster) {
    switch (cluster->getOFED()) {
        case Cluster::OFED::None:
            return;
        case Cluster::OFED::Inbox:
            runCommand("dnf -y groupinstall \"Infiniband Support\"");
            break;
        case Cluster::OFED::Mellanox:
            /* TODO: Implement MLNX OFED support */
            runCommand("dnf -y groupinstall \"Infiniband Support\"");
            break;
        default:
            throw; /* Unsupported OFED stack */
    }
}

/* TODO: Restrict by networks */
void Shell::configureNetworkFileSystem () {
#ifdef _DUMMY_
    std::string_view filename = "chroot/etc/exports";
#else
    std::string_view filename = "/etc/exports";
#endif

    cloyster::addStringToFile(filename,
                      "/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
                      "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\n");

    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Shell::removeMemlockLimits () {
#ifdef _DUMMY_
    std::string_view filename = "chroot/etc/security/limits.conf";
#else
    std::string_view filename = "/etc/security/limits.conf";
#endif

    cloyster::addStringToFile(filename, "* soft memlock unlimited\n"
                                        "* hard memlock unlimited\n");
}

/* TODO: Third party libraries and some logic to install stacks according to
 *  specifics of the system: Infiniband, PSM, etc.
 */
void Shell::installDevelopmentComponents () {
    runCommand("dnf -y install ohpc-autotools hwloc-ohpc spack-ohpc"
               "valgrind-ohpc");

    /* Compiler and MPI stacks */
    runCommand("dnf -y install openmpi4-gnu9-ohpc mpich-ofi-gnu9-ohpc"
               "mpich-ucx-gnu9-ohpc mvapich2-gnu9-ohpc");

    /* Default OpenHPC environment */
    runCommand("dnf -y install lmod-defaults-gnu9-openmpi4-ohpc");
}

/* This method is the entrypoint of shell based cluster install
 * The first session of the method will configure and install services on the
 * headnode. The last part will do provisioner related settings and image
 * creation for network booting
 */
void Shell::install(const std::unique_ptr<Cluster>& cluster) {
    configureSELinuxMode(cluster->getSELinux());
    configureFirewall(cluster->isFirewall());
    configureFQDN(cluster->getHeadnode().getFQDN());

    configureHostsFile(
            cluster->getHeadnode()
                        .getConnection(Network::Profile::External)
                        .getAddress(),
            cluster->getHeadnode().getFQDN(),
            cluster->getHeadnode().getHostname());
    configureTimezone(cluster->getTimezone());
    configureLocale(cluster->getLocale());

    configureNetworks(cluster);
    runSystemUpdate(cluster->isUpdateSystem());

    configureTimeService();

    configureRepositories(cluster);
    runSystemUpdate(cluster->isUpdateSystem());

    installRequiredPackages();
    installOpenHPCBase();

    configureInfiniband(cluster);
    configureNetworkFileSystem();

    configureQueueSystem(cluster);
    removeMemlockLimits();

    installDevelopmentComponents();

    //std::unique_ptr<Provisioner> provisioner;
    std::unique_ptr<XCAT> provisioner;
    switch (cluster->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            provisioner = std::make_unique<XCAT>();
            break;
    }

    provisioner->configureRepositories();
    provisioner->installPackages();
    provisioner->setup(cluster);
    provisioner->createImage(cluster, "/root/OracleLinux-R8-U4-x86_64-dvd.iso");
    provisioner->addNodes(cluster);
    provisioner->setNodesImage();
}
