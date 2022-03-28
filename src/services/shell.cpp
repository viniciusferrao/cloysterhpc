//
// Created by Vinícius Ferrão on 31/10/21.
//
#include "shell.h"
#include "xcat.h"
#include "../functions.h"
#include "log.h"

#include <memory>
#include <fmt/format.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/process.hpp>

#include "../cluster.h"

using cloyster::runCommand;

Shell::Shell(const std::unique_ptr<Cluster> &cluster)
            : m_cluster(cluster) {}

void Shell::disableSELinux() {
    runCommand("setenforce 0");

    const auto filename = CHROOT"/etc/sysconfig/selinux";

    cloyster::backupFile(filename);
    cloyster::changeValueInConfigurationFile(filename, "SELINUX", "disabled");
}

void Shell::configureSELinuxMode() {
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

/* TODO: Better implementation */
void Shell::configureFirewall() {
    if (m_cluster->isFirewall())
        runCommand("systemctl enable --now firewalld");
    else
        runCommand("systemctl disable --now firewalld");
}

void Shell::configureFQDN() {
    runCommand(fmt::format("hostnamectl set hostname {}",
                           m_cluster->getHeadnode().getFQDN()));
}

// TODO: Proper file parsing
void Shell::configureHostsFile() {
    auto& headnode = m_cluster->getHeadnode();

    const auto& ip = headnode.getConnection(Network::Profile::External).getAddress();
    const auto& fqdn = headnode.getFQDN();
    const auto& hostname = headnode.getHostname();

    std::string_view filename = CHROOT"/etc/hosts";

    cloyster::addStringToFile(filename,
                            fmt::format("{}\t{} {}\n", ip, fqdn, hostname));
}

void Shell::configureTimezone() {
    runCommand(fmt::format("timedatectl set timezone {}",
                           m_cluster->getTimezone().getTimezone()));
}

void Shell::configureLocale() {
    runCommand(fmt::format("localectl set locale {}", m_cluster->getLocale()));
}

void Shell::disableNetworkManagerDNSOverride() {
    std::string_view filename =
            CHROOT"/etc/NetworkManager/conf.d/90-dns-none.conf";

    // TODO: Would be better handled with a .conf function
    cloyster::addStringToFile(filename, "[main]\n"
                                        "dns=none\n");

    runCommand("systemctl restart NetworkManager");
}

/* This function configure host networks at once with NetworkManager.
 * We enforce that NM is running enabling it with --now and then set default
 * settings and addresses based on data available on the model.
 * At the end of execution we disable DNS override since the headnode machine
 * will be providing the service.
 */
void Shell::configureNetworks(const std::list<Connection>& connections) {
    runCommand("systemctl enable --now NetworkManager");

    for (auto const& connection : std::as_const(connections)) {
        /* For now, we just skip the external network to avoid disconnects */
        if (connection.getNetwork().getProfile() == Network::Profile::External)
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
                magic_enum::enum_name(
                        connection.getNetwork().getProfile()),
                interface,
                magic_enum::enum_name(
                        connection.getNetwork().getType()),
                connection.getAddress(),
                connection.getNetwork().cidr.at(
                        connection.getNetwork().getSubnetMask()),
                connection.getNetwork().getGateway(),
                fmt::join(connection.getNetwork().getNameserver(), " "),
                connection.getNetwork().getDomainName()));
    }

    disableNetworkManagerDNSOverride();
}

void Shell::runSystemUpdate() {
    if (m_cluster->isUpdateSystem())
        runCommand("dnf -y update");
}

void Shell::installRequiredPackages() {
    runCommand("dnf -y install wget dnf-plugins-core");
}

void Shell::configureRepositories() {
    runCommand("dnf -y install "
               "https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install "
               "https://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm");

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

void Shell::installOpenHPCBase() {
    runCommand("dnf -y install ohpc-base");
}

void Shell::configureTimeService (const std::list<Connection>& connections) {
    if (runCommand("rpm -q chrony"))
        runCommand("dnf -y install chrony");

    std::string_view filename = CHROOT"/etc/chrony.conf";

    for (const auto& connection : std::as_const(connections)) {
        if ((connection.getNetwork().getProfile() ==
             Network::Profile::Management) ||
            (connection.getNetwork().getProfile() ==
             Network::Profile::Service)) {

            cloyster::addStringToFile(
                    filename,
                    fmt::format("allow {}/{}\n",
                              connection.getAddress(),
                              connection.getNetwork().cidr.at(
                                  connection.getNetwork().getSubnetMask())));
        }
    }

    runCommand("systemctl enable --now chronyd");
}

void Shell::configureQueueSystem() {
    if (const auto& queue = m_cluster->getQueueSystem()) {
        switch (queue.value()->getKind()) {
            case QueueSystem::Kind::None: {
                __builtin_unreachable();
            }

            case QueueSystem::Kind::SLURM: {
                runCommand("dnf -y install ohpc-slurm-server");
                // TODO: Use std::filesystem
                //  std::filesystem::copy_file("/etc/slurm/slurm.conf.ohpc",
                //                             "/etc/slurm/slurm.conf");
                runCommand("cp /etc/slurm/slurm.conf.ohpc /etc/slurm/slurm.conf");
                runCommand(fmt::format("perl -pi -e "
                                       "\"s/ControlMachine=\\S+/ControlMachine={}/\" "
                                       "/etc/slurm/slurm.conf",
                                       m_cluster->getHeadnode().getFQDN()));
                runCommand("systemctl enable --now munge");
                runCommand("systemctl enable --now slurmctld");
                break;
            }

            case QueueSystem::Kind::PBS: {
                const auto &pbs = dynamic_cast<PBS*>(queue.value().get());

                runCommand("dnf -y install openpbs-server-ohpc");
                runCommand("systemctl enable --now pbs");
                runCommand("qmgr -c \"set server default_qsub_arguments= -V\"");
                runCommand(fmt::format("qmgr -c \"set server resources_default.place={}\"",
                                       magic_enum::enum_name<PBS::ExecutionPlace>(
                                               pbs->getExecutionPlace())));
                runCommand("qmgr -c \"set server job_history_enable=True\"");
                break;
            }
        }
    }
}

void Shell::configureInfiniband() {
    switch (m_cluster->getOFED()) {
        case Cluster::OFED::None:
            return;
        case Cluster::OFED::Inbox:
            runCommand("dnf -y groupinstall \"Infiniband Support\"");
            break;
        case Cluster::OFED::Mellanox:
            /* TODO: Implement MLNX OFED support */
            throw std::logic_error("MLNX OFED is not yet supported");
        case Cluster::OFED::Oracle:
            /* TODO: Implement Oracle RDMA release */
            throw std::logic_error("Oracle RDMA release is not yet supported");
    }
}

/* TODO: Restrict by networks */
void Shell::configureNetworkFileSystem() {
    std::string_view filename = CHROOT"/etc/exports";

    cloyster::addStringToFile(filename,
                      "/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
                      "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)\n");

    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");
}

void Shell::removeMemlockLimits() {
    std::string_view filename = CHROOT"/etc/security/limits.conf";

    cloyster::addStringToFile(filename, "* soft memlock unlimited\n"
                                        "* hard memlock unlimited\n");
}

/* TODO: Third party libraries and some logic to install stacks according to
 *  specifics of the system: Infiniband, PSM, etc.
 */
void Shell::installDevelopmentComponents() {
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
void Shell::install() {
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

    //std::unique_ptr<Provisioner> provisioner;
    std::unique_ptr<XCAT> provisioner;
    switch (m_cluster->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            provisioner = std::make_unique<XCAT>();
            break;
    }

    provisioner->configureRepositories();
    provisioner->installPackages();
    provisioner->setup(m_cluster);
    provisioner->createImage(m_cluster);
    provisioner->addNodes(m_cluster);
    provisioner->setNodesImage();
}
