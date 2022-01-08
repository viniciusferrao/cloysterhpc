/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "xcat.h"
#include "execution.h"
#include "shell.h"
#include "../functions.h"

#include <fmt/format.h>

/* TODO: Implement a repos class to properly do this */
void XCAT::configureRepositories() {
    Shell::runCommand("wget -P /etc/yum.repos.d "
                      "https://xcat.org/files/xcat/repos/yum/latest/xcat-core/xcat-core.repo");
    Shell::runCommand("wget -P /etc/yum.repos.d "
                      "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/rh8/x86_64/xcat-dep.repo");
}

void XCAT::installPackages () {
    Shell::runCommand("dnf -y install xCAT");
}

void XCAT::setup(const std::unique_ptr<Cluster>& cluster) {
    setDHCPInterfaces(
            cluster->getHeadnode()
                    .getConnection(Network::Profile::Management)
                    .getInterface());
    setDomain(cluster->getDomainName());
}

/* TODO: Maybe create a chdef method to do it cleaner? */
void XCAT::setDHCPInterfaces (std::string_view interface) {
    Shell::runCommand(fmt::format(
            "chdef -t site dhcpinterfaces=\"xcatmn|{}\"", interface));
}

void XCAT::setDomain (std::string_view domain) {
    Shell::runCommand(fmt::format("chdef -t site domain={}", domain));
}

void XCAT::copycds (const std::filesystem::path& isopath) {
    Shell::runCommand(fmt::format("copycds {}", isopath.string()));
}

void XCAT::genimage () {
    Shell::runCommand(fmt::format("genimage {}", m_stateless.osimage));
}

void XCAT::packimage () {
    Shell::runCommand(fmt::format("packimage {}", m_stateless.osimage));
}

void XCAT::nodeset() {
    Shell::runCommand(fmt::format(
            "nodeset compute osimage={}", m_stateless.osimage));
}

void XCAT::createDirectoryTree () {
#ifdef _DUMMY_
    std::filesystem::create_directories("chroot/install/custom/netboot");
#else
    std::filesystem::create_directories("/install/custom/netboot");
#endif
}

void XCAT::configureOpenHPC() {
    const auto packages = {
            "ohpc-base-compute",
            "lmod-ohpc",
            "lua"
    };

    m_stateless.otherpkgs.reserve(packages.size());
    for (const auto& package : std::as_const(packages)) {
        m_stateless.otherpkgs.emplace_back(package);
    }

    // We always sync local Unix files to keep services consistent, even with
    // external directory services
    m_stateless.synclists.emplace_back(
            "/etc/passwd -> /etc/passwd\n"
            "/etc/group -> /etc/group\n"
            "/etc/shadow -> /etc/shadow\n");
}

void XCAT::configureTimeService(const std::unique_ptr<Cluster>& cluster) {
    m_stateless.otherpkgs.emplace_back("chrony");

    m_stateless.postinstall.emplace_back(fmt::format(
            "echo \"server {}\" >> $installroot/etc/chrony.conf\n\n",
            cluster->getHeadnode()
                    .getConnection(Network::Profile::Management)
                    .getAddress()));
}

void XCAT::configureSLURM (const std::unique_ptr<Cluster>& cluster) {
    m_stateless.otherpkgs.emplace_back("ohpc-slurm-client");

    m_stateless.postinstall.emplace_back(fmt::format(
            "echo SLURMD_OPTIONS=\"--conf-server {}\" > "
            "$installroot/etc/sysconfig/slurmd\n\n",
            cluster->getHeadnode()
                    .getConnection(Network::Profile::Management)
                    .getAddress()));

    /* TODO: Enable "if" disallow login on compute nodes */
    m_stateless.postinstall.emplace_back(
            "echo \"account required pam_slurm.so\" >> "
            "$installroot/etc/pam.d/sshd\n"
            "\n");

    m_stateless.synclists.emplace_back(
            "/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf\n"
            "/etc/munge/munge.key -> /etc/munge/munge.key\n"
            "\n");
}

void XCAT::generateOtherPkgListFile () {
#ifdef _DUMMY_
    std::string_view filename =
            "chroot/install/custom/netboot/compute.otherpkglist";
#else
    std::string_view filename = "/install/custom/netboot/compute.otherpkglist";
#endif

    cloyster::addStringToFile(filename, fmt::format("{}\n",
                        fmt::join(m_stateless.otherpkgs, "\n")));

}

void XCAT::generatePostinstallFile (const std::unique_ptr<Cluster>& cluster) {
#ifdef _DUMMY_
    std::string_view filename =
            "chroot/install/custom/netboot/compute.postinstall";
#else
    std::string_view filename = "/install/custom/netboot/compute.postinstall";
#endif
    m_stateless.postinstall.emplace_back(fmt::format(
        "cat << END >> $installroot/etc/fstab\n"
        "{0}:/home /home nfs nfsvers=3,nodev,nosuid 0 0\n"
        "{0}:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3,nodev 0 0\n"
        "END\n\n", cluster->getHeadnode()
                    .getConnection(Network::Profile::Management)
                    .getAddress()));

    m_stateless.postinstall.emplace_back(
        "perl -pi -e 's/# End of file/\\* soft memlock unlimited\\n$&/s' "
        "$installroot/etc/security/limits.conf\n"
        "perl -pi -e 's/# End of file/\\* hard memlock unlimited\\n$&/s' "
        "$installroot/etc/security/limits.conf\n"
        "\n");

    m_stateless.postinstall.emplace_back("systemctl disable firewalld\n");

    for (const auto& entries : std::as_const(m_stateless.postinstall)) {
        cloyster::addStringToFile(filename, entries);
    }
}

void XCAT::generateSynclistsFile () {
#ifdef _DUMMY_
    std::string_view filename =
            "chroot/install/custom/netboot/compute.synclists";
#else
    std::string_view filename = "/install/custom/netboot/compute.synclists";
#endif
    cloyster::addStringToFile(filename,
                              "/etc/passwd -> /etc/passwd\n"
                              "/etc/group -> /etc/group\n"
                              "/etc/shadow -> /etc/shadow\n"
                              "/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf\n"
                              "/etc/munge/munge.key -> /etc/munge/munge.key\n");
}

void XCAT::configureOSImageDefinition(const std::unique_ptr<Cluster>& cluster) {
    Shell::runCommand(fmt::format(
            "chdef -t osimage {} --plus otherpkglist="
            "/install/custom/netboot/compute.pkglist", m_stateless.osimage));

    Shell::runCommand(fmt::format(
            "chdef -t osimage {} --plus postinstall="
            "/install/custom/netboot/compute.postinstall", m_stateless.osimage));

    Shell::runCommand(fmt::format(
            "chdef -t osimage {} --plus synclists="
            "/install/custom/netboot/compute.synclists", m_stateless.osimage));

    /* Add external repositories to otherpkgdir */
    /* TODO: Fix repos to EL8
     *  - Repos URL may be generated with OS class methods
     *     OS.getArch(); OS.getVersion();
     */
    std::vector<std::string_view> repos;

    switch (cluster->getHeadnode().getOS().getDistro()) {
        case OS::Distro::RHEL:
            repos.emplace_back(
                    "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/baseos/os");
            repos.emplace_back(
                    "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/appstream/os");
            repos.emplace_back(
                    "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/codeready-builder/os");
            break;
        case OS::Distro::OL:
            repos.emplace_back(
                    "https://yum.oracle.com/repo/OracleLinux/OL8/baseos/latest/x86_64");
            repos.emplace_back(
                    "https://yum.oracle.com/repo/OracleLinux/OL8/appstream/x86_64");
            repos.emplace_back(
                    "https://yum.oracle.com/repo/OracleLinux/OL8/codeready/builder/x86_64");
            repos.emplace_back(
                    "https://yum.oracle.com/repo/OracleLinux/OL8/UEKR6/x86_64");
            break;
    }

    repos.emplace_back("https://download.fedoraproject.org/pub/epel/8/Everything/x86_64");

    /* TODO: if OpenHPC statement */
    repos.emplace_back("https://repos.openhpc.community/OpenHPC/2/CentOS_8");
    repos.emplace_back("https://repos.openhpc.community/OpenHPC/2/updates/CentOS_8");


    Shell::runCommand(fmt::format(
            "chdef -t osimage {} --plus otherpkgdir={}",
            m_stateless.osimage, fmt::join(repos, ",")));
}

void XCAT::customizeImage () {
    // Bugfixes for Munge
    Shell::runCommand(fmt::format(
            "chroot {} chown munge:munge /var/lib/munge",
            m_stateless.chroot.string()));
    Shell::runCommand(fmt::format(
            "chroot {} chown munge:munge /etc/munge",
            m_stateless.chroot.string()));
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage (const std::unique_ptr<Cluster>& cluster,
                        ImageType imageType, NodeType nodeType) {
    copycds(cluster->getISOPath());
    generateOSImageName(cluster, imageType, nodeType);
    generateOSImagePath(cluster, imageType, nodeType);

    createDirectoryTree();
    configureOpenHPC();
    configureTimeService(cluster);
    configureSLURM(cluster);

    generateOtherPkgListFile();
    generatePostinstallFile(cluster);
    generateSynclistsFile();

    configureOSImageDefinition(cluster);

    genimage();
    customizeImage();
    packimage();
}

void XCAT::addNode(std::string_view t_name, std::string_view t_arch,
                   std::string_view t_address, std::string_view t_macaddress,
                   std::string_view t_bmcaddress,
                   std::string_view t_bmcusername,
                   std::string_view t_bmcpassword) {

    Shell::runCommand(fmt::format(
            "mkdef -f -t node {} arch={} ip={} mac={} bmc={} bmcusername={} "
            "bmcpassword={} mgt=ipmi cons=ipmi serialport=0 serialspeed=115200 "
            "groups=compute,all netboot=xnba", t_name, t_arch, t_address,
            t_macaddress, t_bmcaddress, t_bmcusername, t_bmcpassword));
}

void XCAT::addNodes(const std::unique_ptr<Cluster>& cluster) {
    for (const auto& node : cluster->getNodes())
        addNode(node.getHostname(), "x86_64",
                node.getConnection().front().getAddress(),
                node.getConnection().front().getMAC(),
                node.getBMCAddress(), node.getBMCUsername(),
                node.getBMCPassword());

    // TODO: Create separate functions
    Shell::runCommand("makehosts");
    Shell::runCommand("makedhcp -n");
    Shell::runCommand("makedns -n");
    Shell::runCommand("makegocons");
    nodeset();
}

void XCAT::setNodesImage() {
    nodeset();
}

void XCAT::generateOSImageName(const std::unique_ptr<Cluster>& cluster,
                               ImageType imageType, NodeType nodeType) {
    std::string osimage;

    switch(cluster->getHeadnode().getOS().getDistro()) {
        case OS::Distro::RHEL:
            osimage += "rhels";
            osimage += cluster->getHeadnode().getOS().getVersion();
            break;
        case OS::Distro::OL:
            osimage += "ol";
            osimage += cluster->getHeadnode().getOS().getVersion();
            osimage += ".0";
            break;
    }
    osimage += "-";

    switch (cluster->getHeadnode().getOS().getArch()) {
        case OS::Arch::x86_64:
            osimage += "x86_64";
            break;
        case OS::Arch::ppc64le:
            osimage += "ppc64le";
            break;
    }
    osimage += "-";

    switch (imageType) {
        case ImageType::Install:
            osimage += "install";
            break;
        case ImageType::Netboot:
            osimage += "netboot";
            break;
    }
    osimage += "-";

    switch (nodeType) {
        case NodeType::Compute:
            osimage += "compute";
            break;
        case NodeType::Service:
            osimage += "service";
            break;
    }

    m_stateless.osimage = osimage;
}

void XCAT::generateOSImagePath(const std::unique_ptr<Cluster>& cluster,
                               ImageType imageType, NodeType nodeType) {

    if (imageType != XCAT::ImageType::Netboot)
        throw std::logic_error(
                "Image path is only available on Netboot (Stateless) images");

    std::filesystem::path chroot = "/install/netboot/";

    switch(cluster->getHeadnode().getOS().getDistro()) {
        case OS::Distro::RHEL:
            chroot += "rhels";
            chroot += cluster->getHeadnode().getOS().getVersion();
            break;
        case OS::Distro::OL:
            chroot += "ol";
            chroot += cluster->getHeadnode().getOS().getVersion();
            chroot += ".0";
            break;
    }

    chroot += "/x86_64/compute/rootimg";
    m_stateless.chroot = chroot;
}
