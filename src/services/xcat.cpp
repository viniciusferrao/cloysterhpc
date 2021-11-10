/* This file contains just placeholder implementations for future reference.
 * Everything here must be (re)factored.
 */

#include "xcat.h"
#include "execution.h"
#include "shell.h"
#include "../functions.h"

#include <filesystem>
#include <fmt/format.h>

XCAT::XCAT(Shell& executionEngine)
    : m_executionEngine(executionEngine) {}

XCAT::~XCAT() = default;

void XCAT::configureRepositories() {
    m_executionEngine.runCommand("wget -P /etc/yum.repos.d "
                                 "https://xcat.org/files/xcat/repos/yum/latest/xcat-core/xcat-core.repo");
    m_executionEngine.runCommand("wget -P /etc/yum.repos.d "
                                 "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/rh8/x86_64/xcat-dep.repo");
}

void XCAT::installPackages () {
    m_executionEngine.runCommand("dnf -y install xCAT");
}

void XCAT::setDHCPInterfaces (std::string_view interface) {
    auto command = fmt::format(
            "chdef -t site dhcpinterfaces=\"xcatmn|{}\"", interface);
    
    m_executionEngine.runCommand(command);
}

void XCAT::setDomain (std::string_view domain) {
    auto command = fmt::format("chdef -t site domain={}", domain);

    m_executionEngine.runCommand(command);
}

void XCAT::copycds (std::string_view isopath) {
    auto command = fmt::format("copycds {}", isopath);

    m_executionEngine.runCommand(command);
}

void XCAT::genimage (std::string_view osimage) {
    auto command = fmt::format("genimage {}", osimage);

    m_executionEngine.runCommand(command);
}

void XCAT::packimage (std::string_view osimage) {
    auto command = fmt::format("packimage {}", osimage);

    m_executionEngine.runCommand(command);
}

void XCAT::createDirectoryTree () {
#ifdef _DUMMY_
    std::filesystem::create_directories("chroot/install/custom/netboot");
#else
    std::filesystem::create_directories("/install/custom/netboot");
#endif
}

void XCAT::generateOtherPkgListFile () {
    auto packages = {
        "ohpc-base-compute",
        "ohpc-slurm-client",
        "kernel",
        "chrony",
        "lmod-ohpc",
        "lua"
    };

#ifdef _DUMMY_
    std::string_view filename =
            "chroot/install/custom/netboot/compute.otherpkglist";
#else
    std::string_view filename = "/install/custom/netboot/compute.otherpkglist";
#endif

    cloyster::addStringToFile(filename,
                  fmt::format("{}", fmt::join(packages, "\n")));
}

/* TODO: Replace hardcoded IP addresses at end of fmt::format(); */
void XCAT::generatePostinstallFile () {
#ifdef _DUMMY_
    std::string_view filename =
            "chroot/install/custom/netboot/compute.postinstall";
#else
    std::string_view filename = "/install/custom/netboot/compute.postinstall";
#endif

    cloyster::addStringToFile(filename, fmt::format(
        "#!/bin/sh\n"
        "\n"
        "cat << END >> $installroot/etc/fstab\n"
        "{}:/home /home nfs nfsvers=3,nodev,nosuid 0 0\n"
        "{}:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3,nodev 0 0\n"
        "END\n"
        "\n"
        "perl -pi -e 's/# End of file/\\* soft memlock unlimited\\n$&/s' "
        "$installroot/etc/security/limits.conf\n"
        "perl -pi -e 's/# End of file/\\* hard memlock unlimited\\n$&/s' "
        "$installroot/etc/security/limits.conf\n"
        "\n"
        "echo \"account required pam_slurm.so\" >> "
        "$installroot/etc/pam.d/sshd\n",
        "10.20.0.1", "10.20.0.1"));
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

void XCAT::configureOSImageDefinition (std::string_view osimage) {
    m_executionEngine.runCommand(fmt::format(
            "chdef -t osimage {} --plus otherpkglist="
            "/install/custom/netboot/compute.pkglist", osimage));

    m_executionEngine.runCommand(fmt::format(
            "chdef -t osimage {} --plus postinstall="
            "/install/custom/netboot/compute.postinstall", osimage));

    m_executionEngine.runCommand(fmt::format(
            "chdef -t osimage {} --plus synclists="
            "/install/custom/netboot/compute.synclists", osimage));

    /* Add external repositories to otherpkgdir */
    /* TODO: Fix repos to EL8
     *  - Repos URL may be generated with OS class methods
     *     OS.getArch(); OS.getVersion();
     *  - Missing distro detection
     */
    m_executionEngine.runCommand(fmt::format(
        "chdef -t osimage {} --plus otherpkgdir="
        "http://repos.openhpc.community/OpenHPC/2/CentOS_8,"
        "http://repos.openhpc.community/OpenHPC/2/updates/CentOS_8,"
        "https://yum.oracle.com/repo/OracleLinux/OL8/baseos/latest/x86_64,"
        "https://yum.oracle.com/repo/OracleLinux/OL8/appstream/x86_64,"
        "https://yum.oracle.com/repo/OracleLinux/OL8/codeready/builder/x86_64,"
        "https://yum.oracle.com/repo/OracleLinux/OL8/UEKR6/x86_64,"
        "https://download.fedoraproject.org/pub/epel/8/Everything/x86_64",
        osimage));
}

/* This should be on Postinstall instead */
void XCAT::customizeImage () {
    m_executionEngine.runCommand(fmt::format(
            "chroot {} systemctl disable firewalld", "CHROOT_DIR"));

    m_executionEngine.runCommand(fmt::format(
        "echo SLURMD_OPTIONS=\"--conf-server {}\" > "
            "$CHROOT/etc/sysconfig/slurmd", "HEADNODE_IP 10.20.0.1"));

    m_executionEngine.runCommand(fmt::format(
            "echo \"server {}\" >> {}/etc/chrony.conf",
            "HEADNODE_IP", "CHROOT_DIR"));
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage (std::string_view isopath) {
    copycds(isopath);
    createDirectoryTree();
    generateOtherPkgListFile();
    generatePostinstallFile();
    generateSynclistsFile();
    configureOSImageDefinition("ol8.4.0-x86_64-netboot-compute");
    genimage("ol8.4.0-x86_64-netboot-compute");
    customizeImage();
    packimage("ol8.4.0-x86_64-netboot-compute");
}

/* TODO: Remove this function */
void XCAT::addOpenHPCComponents (std::string_view chroot) {
    m_executionEngine.runCommand(fmt::format(
            "cp /etc/yum.repos.d/OpenHPC.repo {}/etc/yum.repos.d", chroot));
    m_executionEngine.runCommand(fmt::format(
            "cp /etc/yum.repos.d/epel.repo {}/etc/yum.repos.d", chroot));
    m_executionEngine.runCommand(fmt::format(
            "dnf -y install --installroot={} install ohpc-base-compute", chroot));
    m_executionEngine.runCommand(fmt::format(
            "chroot {} systemctl disable firewalld", chroot));
}
