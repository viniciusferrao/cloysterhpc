/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "xcat.h"
#include "execution.h"
#include "shell.h"
#include "../functions.h"

#include <filesystem>
#include <fmt/format.h>

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

void XCAT::setDHCPInterfaces (std::string_view interface) {
    auto command = fmt::format(
            "chdef -t site dhcpinterfaces=\"xcatmn|{}\"", interface);
    
    Shell::runCommand(command);
}

void XCAT::setDomain (std::string_view domain) {
    auto command = fmt::format("chdef -t site domain={}", domain);

    Shell::runCommand(command);
}

void XCAT::copycds (std::string_view isopath) {
    auto command = fmt::format("copycds {}", isopath);

    Shell::runCommand(command);
}

void XCAT::genimage () {
    auto command = fmt::format("genimage {}", m_stateless.osimage);

    Shell::runCommand(command);
}

void XCAT::packimage () {
    auto command = fmt::format("packimage {}", m_stateless.osimage);

    Shell::runCommand(command);
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

    m_stateless.postinstall.emplace_back(
            "echo \"account required pam_slurm.so\" >> "
            "$installroot/etc/pam.d/sshd\n"
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
                              "/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf \n"
                              "/etc/munge/munge.key -> /etc/munge/munge.key\n");
}

void XCAT::configureOSImageDefinition () {
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
     *  - Missing distro detection
     */
    Shell::runCommand(fmt::format(
            "chdef -t osimage {} --plus otherpkgdir="
            "http://repos.openhpc.community/OpenHPC/2/CentOS_8,"
            "http://repos.openhpc.community/OpenHPC/2/updates/CentOS_8,"
            "https://yum.oracle.com/repo/OracleLinux/OL8/baseos/latest/x86_64,"
            "https://yum.oracle.com/repo/OracleLinux/OL8/appstream/x86_64,"
            "https://yum.oracle.com/repo/OracleLinux/OL8/codeready/builder/x86_64,"
            "https://yum.oracle.com/repo/OracleLinux/OL8/UEKR6/x86_64,"
            "https://download.fedoraproject.org/pub/epel/8/Everything/x86_64",
            m_stateless.osimage));
}

/* This should be on Postinstall instead */
void XCAT::customizeImage () {
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage (const std::unique_ptr<Cluster>& cluster,
                        std::string_view isopath) {
    copycds(isopath);
    createDirectoryTree();
    configureOpenHPC();
    configureTimeService(cluster);
    configureSLURM(cluster);
    generateOtherPkgListFile();
    generatePostinstallFile(cluster);
    generateSynclistsFile();
    configureOSImageDefinition();
    genimage();
    customizeImage();
    packimage();
}
