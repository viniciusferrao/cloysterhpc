/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/execution.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>

#include <cstdlib> // setenv / getenv
#include <fmt/format.h>

XCAT::XCAT(const std::unique_ptr<Cluster>& cluster)
    : m_cluster(cluster)
{

    // Initialize some environment variables needed by proper xCAT execution
    // TODO: Look for a better way to do this
    std::string oldPath = std::getenv("PATH");
    std::string newPath = "/opt/xcat/bin:"
                          "/opt/xcat/sbin:"
                          "/opt/xcat/share/xcat/tools:"
        + oldPath;
    setenv("PATH", newPath.c_str(), true);
    setenv("XCATROOT", "/opt/xcat", false);

    // TODO: Hacky, we should properly set environment variable on locale
    setenv("PERL_BADLANG", "0", false);
}

void XCAT::installPackages() { cloyster::runCommand("dnf -y install xCAT"); }

void XCAT::setup()
{
    setDHCPInterfaces(m_cluster->getHeadnode()
                          .getConnection(Network::Profile::Management)
                          .getInterface()
                          .value());
    setDomain(m_cluster->getDomainName());
}

/* TODO: Maybe create a chdef method to do it cleaner? */
void XCAT::setDHCPInterfaces(std::string_view interface)
{
    cloyster::runCommand(
        fmt::format("chdef -t site dhcpinterfaces=\"xcatmn|{}\"", interface));
}

void XCAT::setDomain(std::string_view domain)
{
    cloyster::runCommand(fmt::format("chdef -t site domain={}", domain));
}

void XCAT::copycds(const std::filesystem::path& diskImage)
{
    cloyster::runCommand(fmt::format("copycds {}", diskImage.string()));
}

void XCAT::genimage()
{
    cloyster::runCommand(fmt::format("genimage {}", m_stateless.osimage));
}

void XCAT::packimage()
{
    cloyster::runCommand(fmt::format("packimage {}", m_stateless.osimage));
}

void XCAT::nodeset(std::string_view nodes)
{
    cloyster::runCommand(
        fmt::format("nodeset {} osimage={}", nodes, m_stateless.osimage));
}

void XCAT::createDirectoryTree()
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create the directory CHROOT/install/custom/netboot");
        return;
    }

    std::filesystem::create_directories(CHROOT "/install/custom/netboot");
}

void XCAT::configureOpenHPC()
{
    const auto packages = { "ohpc-base-compute", "lmod-ohpc", "lua" };

    m_stateless.otherpkgs.reserve(packages.size());
    for (const auto& package : std::as_const(packages)) {
        m_stateless.otherpkgs.emplace_back(package);
    }

    // We always sync local Unix files to keep services consistent, even with
    // external directory services
    m_stateless.synclists.emplace_back("/etc/passwd -> /etc/passwd\n"
                                       "/etc/group -> /etc/group\n"
                                       "/etc/shadow -> /etc/shadow\n");
}

void XCAT::configureTimeService()
{
    m_stateless.otherpkgs.emplace_back("chrony");

    m_stateless.postinstall.emplace_back(fmt::format(
        "echo \"server {} iburst\" >> $IMG_ROOTIMGDIR/etc/chrony.conf\n\n",
        m_cluster->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getAddress()
            .to_string()));
}

void XCAT::configureInfiniband()
{
    if (const auto& ofed = m_cluster->getOFED())
        switch (ofed->getKind()) {
            case OFED::Kind::Inbox:
                m_stateless.otherpkgs.emplace_back("@infiniband");

                break;

            case OFED::Kind::Mellanox:
                throw std::logic_error("MLNX OFED is not yet supported");

                break;

            case OFED::Kind::Oracle:
                throw std::logic_error(
                    "Oracle RDMA release is not yet supported");

                break;
        }
}

void XCAT::configureSLURM()
{
    m_stateless.otherpkgs.emplace_back("ohpc-slurm-client");

    // TODO: Deprecate this for SRV entries on DNS: _slurmctld._tcp 0 100 6817
    m_stateless.postinstall.emplace_back(
        fmt::format("echo SLURMD_OPTIONS=\\\"--conf-server {}\\\" > "
                    "$IMG_ROOTIMGDIR/etc/sysconfig/slurmd\n\n",
            m_cluster->getHeadnode()
                .getConnection(Network::Profile::Management)
                .getAddress()
                .to_string()));

    // TODO: Enable "if" disallow login on compute nodes
    // TODO: Consider pam_slurm_adopt.so
    //  * https://github.com/openhpc/ohpc/issues/1022
    //  * https://slurm.schedmd.com/pam_slurm_adopt.html
    m_stateless.postinstall.emplace_back(
        "echo \"# Block queue evasion\" >> "
        "$IMG_ROOTIMGDIR/etc/pam.d/sshd\n"
        "echo \"account    required     pam_slurm.so\" >> "
        "$IMG_ROOTIMGDIR/etc/pam.d/sshd\n"
        "\n");

    // Enable services on image
    m_stateless.postinstall.emplace_back(
        "chroot $IMG_ROOTIMGDIR systemctl enable munge\n"
        "chroot $IMG_ROOTIMGDIR systemctl enable slurmd\n"
        "\n");

    m_stateless.synclists.emplace_back(
        // Stateless config: we don't need slurm.conf to be synced.
        //"/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf\n"
        "/etc/munge/munge.key -> /etc/munge/munge.key\n"
        "\n");
}

void XCAT::generateOtherPkgListFile()
{
    std::string_view filename
        = CHROOT "/install/custom/netboot/compute.otherpkglist";

    cloyster::removeFile(filename);
    cloyster::addStringToFile(
        filename, fmt::format("{}\n", fmt::join(m_stateless.otherpkgs, "\n")));
}

void XCAT::generatePostinstallFile()
{
    std::string_view filename
        = CHROOT "/install/custom/netboot/compute.postinstall";

    cloyster::removeFile(filename);

    m_stateless.postinstall.emplace_back(
        "perl -pi -e 's/# End of file/\\* soft memlock unlimited\\n$&/s' "
        "$IMG_ROOTIMGDIR/etc/security/limits.conf\n"
        "perl -pi -e 's/# End of file/\\* hard memlock unlimited\\n$&/s' "
        "$IMG_ROOTIMGDIR/etc/security/limits.conf\n"
        "\n");

    m_stateless.postinstall.emplace_back("systemctl disable firewalld\n");

    for (const auto& entries : std::as_const(m_stateless.postinstall)) {
        cloyster::addStringToFile(filename, entries);
    }

    if (cloyster::dryRun) {
        LOG_INFO("Would change file {} permissions", filename);
        return;
    }

    std::filesystem::permissions(filename,
        std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec
            | std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add);
}

void XCAT::generateSynclistsFile()
{
    std::string_view filename
        = CHROOT "/install/custom/netboot/compute.synclists";

    cloyster::removeFile(filename);
    cloyster::addStringToFile(filename,
        "/etc/passwd -> /etc/passwd\n"
        "/etc/group -> /etc/group\n"
        "/etc/shadow -> /etc/shadow\n"
        //"/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf\n"
        "/etc/munge/munge.key -> /etc/munge/munge.key\n");
}

void XCAT::configureOSImageDefinition()
{
    cloyster::runCommand(
        fmt::format("chdef -t osimage {} --plus otherpkglist="
                    "/install/custom/netboot/compute.otherpkglist",
            m_stateless.osimage));

    cloyster::runCommand(
        fmt::format("chdef -t osimage {} --plus postinstall="
                    "/install/custom/netboot/compute.postinstall",
            m_stateless.osimage));

    cloyster::runCommand(
        fmt::format("chdef -t osimage {} --plus synclists="
                    "/install/custom/netboot/compute.synclists",
            m_stateless.osimage));

    /* Add external repositories to otherpkgdir */
    /* TODO: Fix repos to EL8
     *  - Repos URL may be generated with OS class methods
     *     OS.getArch(); OS.getVersion();
     */
    std::vector<std::string_view> repos;

    switch (m_cluster->getNodes()[0].getOS().getDistro()) {
        case OS::Distro::RHEL:
            repos.emplace_back(
                "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/baseos/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/appstream/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/codeready-builder/os");
            break;
        case OS::Distro::OL:
            repos.emplace_back("https://yum.oracle.com/repo/OracleLinux/OL8/"
                               "baseos/latest/x86_64");
            repos.emplace_back(
                "https://yum.oracle.com/repo/OracleLinux/OL8/appstream/x86_64");
            repos.emplace_back("https://yum.oracle.com/repo/OracleLinux/OL8/"
                               "codeready/builder/x86_64");
            repos.emplace_back(
                "https://yum.oracle.com/repo/OracleLinux/OL8/UEKR6/x86_64");
            break;
        case OS::Distro::Rocky:
            repos.emplace_back(
                "http://ftp.unicamp.br/pub/rocky/8/BaseOS/x86_64/os");
            repos.emplace_back(
                "http://ftp.unicamp.br/pub/rocky/8/PowerTools/x86_64/os");
            repos.emplace_back(
                "http://ftp.unicamp.br/pub/rocky/8/AppStream/x86_64/os");
            break;
        case OS::Distro::AlmaLinux:
            repos.emplace_back(
                "https://repo.almalinux.org/almalinux/8/BaseOS/x86_64/os");
            repos.emplace_back("https://repo.almalinux.org/almalinux/8/"
                               "PowerTools/x86_64/os");
            repos.emplace_back(
                "https://repo.almalinux.org/almalinux/8/AppStream/x86_64/os");
            break;
    }

    repos.emplace_back(
        "https://download.fedoraproject.org/pub/epel/8/Everything/x86_64");
    repos.emplace_back(
        "https://download.fedoraproject.org/pub/epel/8/Modular/x86_64");

    /* TODO: if OpenHPC statement */
    repos.emplace_back("http://repos.openhpc.community/OpenHPC/2/CentOS_8");
    repos.emplace_back(
        "http://repos.openhpc.community/OpenHPC/2/updates/CentOS_8");

    cloyster::runCommand(
        fmt::format("chdef -t osimage {} --plus otherpkgdir={}",
            m_stateless.osimage, fmt::join(repos, ",")));
}

void XCAT::customizeImage()
{
    // Bugfixes for Munge
    // Not needed if we sync /etc/passwd and /etc/groups
#if 0
    cloyster::runCommand(fmt::format(
            "/bin/bash -c \"chroot {} chown munge:munge /var/lib/munge\"",
            m_stateless.chroot.string()));
    cloyster::runCommand(fmt::format(
            "/bin/bash -c \"chroot {} chown munge:munge /var/log/munge\"",
            m_stateless.chroot.string()));
    cloyster::runCommand(fmt::format(
            "/bin/bash -c \"chroot {} chown munge:munge /etc/munge\"",
            m_stateless.chroot.string()));
#endif
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage(ImageType imageType, NodeType nodeType)
{
    copycds(m_cluster->getDiskImage());
    generateOSImageName(imageType, nodeType);
    generateOSImagePath(imageType, nodeType);

    createDirectoryTree();
    configureOpenHPC();
    configureTimeService();
    configureInfiniband();
    configureSLURM();

    generateOtherPkgListFile();
    generatePostinstallFile();
    generateSynclistsFile();

    configureOSImageDefinition();

    genimage();
    customizeImage();
    packimage();
}

void XCAT::addNode(const Node& node)
{
    LOG_DEBUG("Adding node {} to xCAT", node.getHostname());

    std::string command = fmt::format(
        "mkdef -f -t node {} arch={} ip={} mac={} groups=compute,all "
        "netboot=xnba ",
        node.getHostname(), magic_enum::enum_name(node.getOS().getArch()),
        node.getConnection(Network::Profile::Management)
            .getAddress()
            .to_string(),
        node.getConnection(Network::Profile::Management).getMAC().value());

    if (const auto& bmc = node.getBMC())
        command += fmt::format("bmc={} bmcusername={} bmcpassword={} mgt=ipmi "
                               "cons=ipmi serialport={} serialspeed={} ",
            bmc->m_address, bmc->m_username, bmc->m_password, bmc->m_serialPort,
            bmc->m_serialSpeed);

    // FIXME:
    //  *********************************************************************
    //  * This is __BAD__ implementation. We cannot use try/catch as return *
    //  *********************************************************************
    try {
        command += fmt::format(
            "nicips.ib0={} nictypes.ib0=\"InfiniBand\" nicnetworks.ib0=ib0 ",
            node.getConnection(Network::Profile::Application)
                .getAddress()
                .to_string());
    } catch (...) {
    }

    cloyster::runCommand(command);
}

void XCAT::addNodes()
{
    for (const auto& node : m_cluster->getNodes())
        addNode(node);

    // TODO: Create separate functions
    cloyster::runCommand("makehosts");
    cloyster::runCommand("makedhcp -n");
    cloyster::runCommand("makedns -n");
    cloyster::runCommand("makegocons");
    setNodesImage();
}

void XCAT::setNodesImage()
{
    // TODO: For now we always run nodeset for all computes
    nodeset("compute");
}

void XCAT::setNodesBoot()
{
    // TODO: Do proper checking if a given node have BMC support, and then issue
    //  rsetboot only on the compatible machines instead of running in compute.
    cloyster::runCommand("rsetboot compute net");
}

void XCAT::resetNodes() { cloyster::runCommand("rpower compute reset"); }

void XCAT::generateOSImageName(ImageType imageType, NodeType nodeType)
{
    std::string osimage;

    // FIXME: If there's no nodes defined this switch will fail, it should
    //  instead generate an image for future use.
    switch (m_cluster->getNodes()[0].getOS().getDistro()) {
        case OS::Distro::RHEL:
            osimage += "rhels";
            osimage += m_cluster->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::OL:
            osimage += "ol";
            osimage += m_cluster->getNodes()[0].getOS().getVersion();
            osimage += ".0";
            break;
        case OS::Distro::Rocky:
            osimage += "rocky";
            osimage += m_cluster->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::AlmaLinux:
            osimage += "alma";
            osimage += m_cluster->getNodes()[0].getOS().getVersion();
            break;
    }
    osimage += "-";

    switch (m_cluster->getNodes()[0].getOS().getArch()) {
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

void XCAT::generateOSImagePath(ImageType imageType, NodeType nodeType)
{

    if (imageType != XCAT::ImageType::Netboot)
        throw std::logic_error(
            "Image path is only available on Netboot (Stateless) images");

    std::filesystem::path chroot = "/install/netboot/";

    switch (m_cluster->getNodes()[0].getOS().getDistro()) {
        case OS::Distro::RHEL:
            chroot += "rhels";
            chroot += m_cluster->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::OL:
            chroot += "ol";
            chroot += m_cluster->getNodes()[0].getOS().getVersion();
            chroot += ".0";
            break;
        case OS::Distro::Rocky:
            chroot += "rocky";
            chroot += m_cluster->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::AlmaLinux:
            chroot += "alma";
            chroot += m_cluster->getNodes()[0].getOS().getVersion();
            break;
    }

    chroot += "/";

    switch (m_cluster->getNodes()[0].getOS().getArch()) {
        case OS::Arch::x86_64:
            chroot += "x86_64";
            break;
        case OS::Arch::ppc64le:
            chroot += "ppc64le";
            break;
    }

    chroot += "/compute/rootimg";
    m_stateless.chroot = chroot;
}
