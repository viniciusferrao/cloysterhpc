/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cstdlib> // setenv / getenv

#include <filesystem>
#include <fmt/format.h>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/execution.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>
#include <variant>

namespace cloyster::services {

using cloyster::models::Node;

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

void XCAT::installPackages()
{
    m_cluster->getHeadnode().getOS().packageManager()->install("initscripts");
    m_cluster->getHeadnode().getOS().packageManager()->install("xCAT");
}

void XCAT::patchInstall()
{
    /* Required for EL 9.5
     * Upstream PR: https://github.com/xcat2/xcat-core/pull/7489
     */
    if (cloyster::runCommand(
            "grep -q \"extensions usr_cert\" "
            "/opt/xcat/share/xcat/scripts/setup-local-client.sh")
        == 0) {
        cloyster::runCommand(
            "sed -i \"s/-extensions usr_cert //g\" "
            "/opt/xcat/share/xcat/scripts/setup-local-client.sh");
        cloyster::runCommand(
            "sed -i \"s/-extensions server //g\" "
            "/opt/xcat/share/xcat/scripts/setup-server-cert.sh");
        cloyster::runCommand("xcatconfig -f");
    } else {
        LOG_WARN("xCAT Already patched, skipping");
    }
}

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

namespace {
    constexpr bool imageExists(const std::string& image)
    {
        LOG_ASSERT(
            image.size() > 0, "Trying to generate an image with empty name");
        std::list<std::string> output;
        int code = cloyster::runCommand(
            fmt::format("lsdef -t osimage {}", image), output);
        if (code == 0 // success
            && (output.size() > 0
                && output.front()
                    != "Could not find any object definitions to display")) {
            LOG_WARN("Skipping image generation {}, use `rmdef -t osimage -o "
                     "{}` to remove "
                     "the image if you want it to be regenerated.",
                image, image);
            LOG_DEBUG("Command output: {}", fmt::join(output, "\n"));
            return true;
        }

        return false;
    }

}; // anonymous namespace

void XCAT::copycds(const std::filesystem::path& diskImage) const
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
    cloyster::createDirectory(CHROOT "/install/custom/netboot");
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

    // TODO: Should be replaced with autofs
    m_stateless.postinstall.emplace_back(
        fmt::format("cat << END >> $IMG_ROOTIMGDIR/etc/fstab\n"
                    "{0}:/home /home nfs nfsvers=3,nodev,nosuid 0 0\n"
                    "{0}:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3,nodev 0 0\n"
                    "END\n\n",
            m_cluster->getHeadnode()
                .getConnection(Network::Profile::Management)
                .getAddress()
                .to_string()));

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
        LOG_WARN("Dry Run: Would change file {} permissions", filename)
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
    auto runner = getRunner();
    runner->executeCommand(
        fmt::format("chdef -t osimage {} --plus otherpkglist="
                    "/install/custom/netboot/compute.otherpkglist",
            m_stateless.osimage));

    runner->executeCommand(
        fmt::format("chdef -t osimage {} --plus postinstall="
                    "/install/custom/netboot/compute.postinstall",
            m_stateless.osimage));

    runner->executeCommand(
        fmt::format("chdef -t osimage {} --plus synclists="
                    "/install/custom/netboot/compute.synclists",
            m_stateless.osimage));

    /* Add external repositories to otherpkgdir */
    std::vector<std::string> repos = getxCATOSImageRepos();

    runner->executeCommand(
        fmt::format("chdef -t osimage {} --plus otherpkgdir={}",
            m_stateless.osimage, fmt::join(repos, ",")));
}

void XCAT::customizeImage()
{
    // Permission fixes for munge
    if (m_cluster->getQueueSystem().value()->getKind()
        == models::QueueSystem::Kind::SLURM) {
        // @TODO This is using the Runner above and cloyster::runCommand here
        //   choose only one!
        cloyster::runCommand(
            fmt::format("cp -f /etc/passwd /etc/group /etc/shadow {}/etc",
                m_stateless.chroot.string()));
        cloyster::runCommand(
            fmt::format("mkdir -p {0}/var/lib/munge {0}/var/log/munge "
                        "{0}/etc/munge {0}/run/munge",
                m_stateless.chroot.string()));
        cloyster::runCommand(fmt::format(
            "chown munge:munge {}/var/lib/munge", m_stateless.chroot.string()));
        cloyster::runCommand(fmt::format(
            "chown munge:munge {}/var/log/munge", m_stateless.chroot.string()));
        cloyster::runCommand(fmt::format(
            "chown munge:munge {}/etc/munge", m_stateless.chroot.string()));
        cloyster::runCommand(fmt::format(
            "chown munge:munge {}/run/munge", m_stateless.chroot.string()));
    }
}

/* This is necessary to avoid problems with EL9-based distros.
 * The xCAT team has discontinued the project and distros based on EL9 are not
 * officially supported by default.
 */
void XCAT::configureEL9()
{
    auto createSymlinkCommand = [](const std::string& folder,
                                    const std::string& version) {
        return fmt::format(
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/compute.rhels9.x86_64.exlist "
            "/opt/xcat/share/xcat/netboot/{0}/compute.{1}.x86_64.exlist,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/compute.rhels9.x86_64.pkglist "
            "/opt/xcat/share/xcat/netboot/{0}/compute.{1}.x86_64.pkglist,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/compute.rhels9.x86_64.postinstall "
            "/opt/xcat/share/xcat/netboot/{0}/compute.{1}.x86_64.postinstall,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/service.rhels9.x86_64.exlist "
            "/opt/xcat/share/xcat/netboot/{0}/service.{1}.x86_64.exlist,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/"
            "service.rhels9.x86_64.otherpkgs.pkglist "
            "/opt/xcat/share/xcat/netboot/{0}/"
            "service.{1}.x86_64.otherpkgs.pkglist,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/service.rhels9.x86_64.pkglist "
            "/opt/xcat/share/xcat/netboot/{0}/service.{1}.x86_64.pkglist,"
            "ln -sf "
            "/opt/xcat/share/xcat/netboot/rh/service.rhels9.x86_64.postinstall "
            "/opt/xcat/share/xcat/netboot/{0}/service.{1}.x86_64.postinstall,"
            "ln -sf /opt/xcat/share/xcat/install/rh/compute.rhels9.pkglist "
            "/opt/xcat/share/xcat/install/{0}/compute.{1}.pkglist,"
            "ln -sf /opt/xcat/share/xcat/install/rh/compute.rhels9.tmpl "
            "/opt/xcat/share/xcat/install/{0}/compute.{1}.tmpl,"
            "ln -sf /opt/xcat/share/xcat/install/rh/service.rhels9.pkglist "
            "/opt/xcat/share/xcat/install/{0}/service.{1}.pkglist,"
            "ln -sf /opt/xcat/share/xcat/install/rh/service.rhels9.tmpl "
            "/opt/xcat/share/xcat/install/{0}/service.{1}.tmpl,"
            "ln -sf "
            "/opt/xcat/share/xcat/install/rh/"
            "service.rhels9.x86_64.otherpkgs.pkglist "
            "/opt/xcat/share/xcat/install/{0}/"
            "service.{1}.x86_64.otherpkgs.pkglist",
            folder, version);
    };

    std::vector<std::string> commands;
    std::vector<std::pair<std::string, std::string>> commandPairs
        = { { "rocky", "rocky9" }, { "ol", "ol9" }, { "alma", "alma9" } };

    for (const auto& pair : commandPairs) {
        std::vector<std::string> temp;
        boost::split(temp, createSymlinkCommand(pair.first, pair.second),
            boost::is_any_of(","));
        commands.insert(commands.end(), temp.begin(), temp.end());
    }

    for (const auto& command : commands) {
        cloyster::runCommand(command);
    }
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage(ImageType imageType, NodeType nodeType)
{
    configureEL9();

    generateOSImageName(imageType, nodeType);

    const auto imageExists_ = imageExists(m_stateless.osimage);
    if (!imageExists_) {
        copycds(m_cluster->getDiskImage().getPath());
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
}

void XCAT::addNode(const Node& node)
{
    LOG_DEBUG("Adding node {} to xCAT", node.getHostname())

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

    switch (m_cluster->getDiskImage().getDistro()) {
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

    if (imageType != XCAT::ImageType::Netboot) {
        throw std::logic_error(
            "Image path is only available on Netboot (Stateless) images");
    }

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

void XCAT::installRepositories()
{
    const std::filesystem::path& repofileDest
        = std::filesystem::temp_directory_path();
    LOG_INFO("Setting up XCAT repositories");
    auto runner = cloyster::getRunner();

    runner->downloadFile("https://xcat.org/files/xcat/repos/yum/devel/"
                         "core-snap/xcat-core.repo",
        repofileDest.string());

    switch (m_cluster->getHeadnode().getOS().getPlatform()) {
        case OS::Platform::el8:
            runner->downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh8/x86_64/xcat-dep.repo",
                repofileDest.string());
            break;
        case OS::Platform::el9:
#ifndef NDEBUG
        // Hack to test EL10 only in debug mode
        case OS::Platform::el10:
#endif
            runner->downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh9/x86_64/xcat-dep.repo",
                repofileDest.string());
            break;
        default:
            throw std::runtime_error("Unsupported platform for xCAT");
    }
    const auto osinf = m_cluster->getHeadnode().getOS();
    const auto repoManager = getRepoManager(osinf);
    for (auto const& dir_entry :
        std::filesystem::directory_iterator { repofileDest }) {
        const auto& path = dir_entry.path();
        if (path.extension() == ".repo") {
            repoManager->install(path);
        }
    }
}

[[deprecated("Refactoring RepoManager, replace the function with the same name "
             "in repo manager")]]
std::vector<std::string> XCAT::getxCATOSImageRepos() const
{
    const auto osinfo = m_cluster->getHeadnode().getOS();
    const auto osArch = magic_enum::enum_name(osinfo.getArch());
    const auto osMajorVersion = osinfo.getMajorVersion();
    const auto osVersion = osinfo.getVersion();

    std::vector<std::string> repos;

    /* FIXME: A LOT OF WORK TO BE DONE HERE BRUH */
    /* BUG: This is a very bad implementation; it should find out the latest
     * version and not be hardcoded. Also the directory formation does not work
     * that way. We should support finding out the Repository paths by parsing
     * /etc/yum.repos.d
     */
    std::vector<std::string> latestEL = { "8.10", "9.5" };

    std::string crb = "CRB";
    std::string rockyBranch
        = "linux"; // To check if Rocky mirror directory points to 'linux'
                   // (latest version) or 'vault'

    // BUG: Really? A string?
    std::string OpenHPCVersion = "3";

    if (osMajorVersion < 9) {
        crb = "PowerTools";
        OpenHPCVersion = "2";
    }

    if (std::ranges::find(latestEL, osVersion) == latestEL.end()) {
        rockyBranch = "vault";
    }

    switch (osinfo.getDistro()) {
        case OS::Distro::RHEL:
            repos.emplace_back(
                "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/baseos/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/appstream/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/codeready-builder/os");
            break;
        case OS::Distro::OL:
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/oracle/{}/"
                            "baseos/latest/{}",
                    osMajorVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/oracle/{}/appstream/{}",
                osMajorVersion, osArch));
            repos.emplace_back(fmt::format("https://mirror.versatushpc.com.br/"
                                           "oracle/{}/codeready/builder/{}",
                osMajorVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/oracle/{}/UEKR7/{}",
                osMajorVersion, osArch));
            break;
        case OS::Distro::Rocky:
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/BaseOS/{}/os",
                rockyBranch, osVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/{}/{}/os",
                rockyBranch, osVersion, crb, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/AppStream/{}/os",
                rockyBranch, osVersion, osArch));
            break;
        case OS::Distro::AlmaLinux:
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/almalinux/"
                            "almalinux/{}/BaseOS/{}/os",
                    osVersion, osArch));
            repos.emplace_back(fmt::format("https://mirror.versatushpc.com.br/"
                                           "almalinux/almalinux/{}/{}/{}/os",
                osVersion, crb, osArch));
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/almalinux/"
                            "almalinux/{}/AppStream/{}/os",
                    osVersion, osArch));
            break;
    }

    repos.emplace_back(
        fmt::format("https://mirror.versatushpc.com.br/epel/{}/Everything/{}",
            osMajorVersion, osArch));

    // Modular repositories are only available on EL8
    if (osMajorVersion == 8) {
        repos.emplace_back(
            fmt::format("https://mirror.versatushpc.com.br/epel/{}/Modular/{}",
                osMajorVersion, osArch));
    }

    repos.emplace_back(
        fmt::format("https://mirror.versatushpc.com.br/openhpc/{}/EL_{}",
            OpenHPCVersion, osMajorVersion));
    repos.emplace_back(fmt::format(
        "https://mirror.versatushpc.com.br/openhpc/{}/updates/EL_{}",
        OpenHPCVersion, osMajorVersion));

    return repos;
}

};
