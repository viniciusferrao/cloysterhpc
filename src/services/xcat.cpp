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
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/xcat.h>

namespace {
using cloyster::models::Cluster;

inline auto cluster() { return cloyster::Singleton<Cluster>::get(); }

// Returns the distribution name with the version, e.g., rocky9.5
std::string getOSImageDistroVersion()
{
    using cloyster::models::OS;
    std::string osimage;

    switch (cluster()->getDiskImage().getDistro()) {
        case OS::Distro::RHEL:
            osimage += "rhels";
            osimage += cluster()->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::OL:
            osimage += "ol";
            osimage += cluster()->getNodes()[0].getOS().getVersion();
            osimage += ".0";
            break;
        case OS::Distro::Rocky:
            osimage += "rocky";
            osimage += cluster()->getNodes()[0].getOS().getVersion();
            break;
        case OS::Distro::AlmaLinux:
            osimage += "alma";
            osimage += cluster()->getNodes()[0].getOS().getVersion();
            break;
    }
    return osimage;
}
}; // namespace{}

namespace cloyster::services {

using cloyster::models::Node;
using cloyster::services::repos::RepoManager;

XCAT::XCAT()
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

    // Ensure image name is setted
    generateOSImageName(ImageType::Netboot, NodeType::Compute);
}

XCAT::Image XCAT::getImage() const { return m_stateless; }

void XCAT::installPackages()
{
    auto osservice = cloyster::Singleton<IOSService>::get();
    osservice->install("initscripts");
    osservice->install("xCAT");
}

void XCAT::patchInstall()
{
    /* Required for EL 9.5
     * Upstream PR: https://github.com/xcat2/xcat-core/pull/7489
     */

    auto runner = cloyster::Singleton<services::IRunner>::get();
    if (runner->executeCommand(
            "grep -q \"extensions usr_cert\" "
            "/opt/xcat/share/xcat/scripts/setup-local-client.sh")
        == 0) {
        runner->executeCommand(
            "sed -i \"s/-extensions usr_cert //g\" "
            "/opt/xcat/share/xcat/scripts/setup-local-client.sh");
        runner->executeCommand(
            "sed -i \"s/-extensions server //g\" "
            "/opt/xcat/share/xcat/scripts/setup-server-cert.sh");
        runner->executeCommand("xcatconfig -f");
    } else {
        LOG_WARN("xCAT Already patched, skipping");
    }
}

void XCAT::setup()
{
    setDHCPInterfaces(cluster()
            ->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getInterface()
            .value());
    setDomain(cluster()->getDomainName());
}

/* TODO: Maybe create a chdef method to do it cleaner? */
void XCAT::setDHCPInterfaces(std::string_view interface)
{
    auto runner = cloyster::Singleton<services::IRunner>::get();
    runner->checkCommand(
        fmt::format("chdef -t site dhcpinterfaces=\"xcatmn|{}\"", interface));
}

void XCAT::setDomain(std::string_view domain)
{
    auto runner = cloyster::Singleton<services::IRunner>::get();
    runner->checkCommand(fmt::format("chdef -t site domain={}", domain));
}

namespace {
    constexpr bool imageExists(const std::string& image)
    {
        LOG_ASSERT(
            image.size() > 0, "Trying to generate an image with empty name");

        auto opts = cloyster::Singleton<cloyster::services::Options>::get();
        if (opts->dryRun) {
            LOG_WARN("Dry-Run: skipping image check, assuming it doesn't exists");
            return false;
        }

        auto runner = cloyster::Singleton<services::IRunner>::get();
        if (opts->shouldForce("genimage")) {
            runner->executeCommand(fmt::format("rmdef -t osimage -o {}", image));
        }

        std::list<std::string> output;
        auto exitCode = runner->executeCommand(fmt::format("lsdef -t osimage {}", image), output);
        if (exitCode == 0) { // image exists
            LOG_WARN("Skipping image generation {}, use --force=genimage to force",
                image, image);
            LOG_DEBUG("Command output: {}", fmt::join(output, "\n"));
            return true;
        }

        return false;
    }

}; // anonymous namespace

void XCAT::copycds(const std::filesystem::path& diskImage) const
{
    cloyster::Singleton<IRunner>::get()->checkCommand(
        fmt::format("copycds {}", diskImage.string()));
}

void XCAT::genimage()
{
    cloyster::Singleton<IRunner>::get()->checkCommand(
        fmt::format("genimage {}", m_stateless.osimage));
}

void XCAT::packimage()
{
    cloyster::Singleton<IRunner>::get()->checkCommand(
        fmt::format("packimage {}", m_stateless.osimage));
}

void XCAT::nodeset(std::string_view nodes)
{
    cloyster::Singleton<IRunner>::get()->checkCommand(
        fmt::format("nodeset {} osimage={}", nodes, m_stateless.osimage));
}

void XCAT::createDirectoryTree()
{
    functions::createDirectory(CHROOT "/install/custom/netboot");
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
        cluster()
            ->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getAddress()
            .to_string()));
}

void XCAT::configureInfiniband()
{
    LOG_INFO("[xCAT] Configuring infiniband");
    if (const auto& ofed = cluster()->getOFED()) {
        switch (ofed->getKind()) {
            case OFED::Kind::Inbox:
                m_stateless.otherpkgs.emplace_back("@infiniband");

                break;

            case OFED::Kind::Mellanox: {
                auto repoManager = cloyster::Singleton<RepoManager>::get();
                auto runner = cloyster::Singleton<IRunner>::get();
                auto arch = cloyster::utils::enums::toString(
                    cluster()->getNodes()[0].getOS().getArch());
                auto osService = cloyster::Singleton<IOSService>::get();
                auto opts = cloyster::Singleton<Options>::get();

                // Add the rpm to the image
                m_stateless.otherpkgs.emplace_back("mlnx-ofa_kernel");
                m_stateless.otherpkgs.emplace_back("doca-ofed");

                // The kernel modules are build by the OFED.cpp module, see
                // OFED.cpp
                const auto kernelVersion = opts->dryRun 
                    ? "5.14.0-503.33.1.el9_5"
                    // getKernelInstalled cannot run at dryRun
                    : osService->getKernelInstalled();
                // Configure Apache to serve the RPM repository
                const auto repoName
                    = fmt::format("doca-kernel-{}", kernelVersion);
                const auto localRepo = functions::createHTTPRepo(repoName);

                // Create the RPM repository
                runner->checkCommand(fmt::format(
                    "bash -c \"cp -v "
                    "/usr/share/doca-host-*/Modules/{}.{}/*.rpm {}\"",
                    kernelVersion, arch, localRepo.directory.string()));
                runner->checkCommand(
                    fmt::format("createrepo {}", localRepo.directory.string()));

                // dryRun does not initialize the repositories
                if (!opts->dryRun) {
                    auto docaUrl = repoManager->repo("doca")->uri().value();
                    runner->checkCommand(fmt::format(
                        "bash -c \"chdef -t osimage {} --plus otherpkgdir={}\"",
                        m_stateless.osimage, docaUrl));
                }

                // Add the local repository to the stateless image
                runner->checkCommand(fmt::format(
                    "bash -c \"chdef -t osimage {} --plus otherpkgdir={}\"",
                    m_stateless.osimage, localRepo.url));

            } break;

            case OFED::Kind::Oracle:
                throw std::logic_error(
                    "Oracle RDMA release is not yet supported");

                break;
        }
    }
}

void XCAT::configureSLURM()
{
    m_stateless.otherpkgs.emplace_back("ohpc-slurm-client");

    // TODO: Deprecate this for SRV entries on DNS: _slurmctld._tcp 0 100 6817
    m_stateless.postinstall.emplace_back(
        fmt::format("echo SLURMD_OPTIONS=\\\"--conf-server {}\\\" > "
                    "$IMG_ROOTIMGDIR/etc/sysconfig/slurmd\n\n",
            cluster()
                ->getHeadnode()
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

    functions::removeFile(filename);
    functions::addStringToFile(
        filename, fmt::format("{}\n", fmt::join(m_stateless.otherpkgs, "\n")));
}

void XCAT::generatePostinstallFile()
{
    std::string_view filename
        = CHROOT "/install/custom/netboot/compute.postinstall";

    functions::removeFile(filename);

    // TODO: Should be replaced with autofs
    m_stateless.postinstall.emplace_back(
        fmt::format("cat << END >> $IMG_ROOTIMGDIR/etc/fstab\n"
                    "{0}:/home /home nfs nfsvers=3,nodev,nosuid 0 0\n"
                    "{0}:/opt/ohpc/pub /opt/ohpc/pub nfs nfsvers=3,nodev 0 0\n"
                    "END\n\n",
            cluster()
                ->getHeadnode()
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
        functions::addStringToFile(filename, entries);
    }

    auto opts = cloyster::Singleton<cloyster::services::Options>::get();

    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would change file {} permissions", filename)
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

    functions::removeFile(filename);
    functions::addStringToFile(filename,
        "/etc/passwd -> /etc/passwd\n"
        "/etc/group -> /etc/group\n"
        "/etc/shadow -> /etc/shadow\n"
        //"/etc/slurm/slurm.conf -> /etc/slurm/slurm.conf\n"
        "/etc/munge/munge.key -> /etc/munge/munge.key\n");
}

void XCAT::configureOSImageDefinition()
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    auto runner = cloyster::Singleton<IRunner>::get();
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
    if (!opts->dryRun) {
        std::vector<std::string> repos = getxCATOSImageRepos();
        runner->executeCommand(
            fmt::format("chdef -t osimage {} --plus otherpkgdir={}",
                m_stateless.osimage, fmt::join(repos, ",")));
    }
}

void XCAT::customizeImage(const std::vector<ScriptBuilder>& customizations)
{
    auto runner = cloyster::Singleton<IRunner>::get();
    // @TODO: Extract the munge fixes to its own customization script
    // Permission fixes for munge
    if (cluster()->getQueueSystem().value()->getKind()
        == models::QueueSystem::Kind::SLURM) {
        runner->executeCommand(
            fmt::format("cp -f /etc/passwd /etc/group /etc/shadow {}/etc",
                m_stateless.chroot.string()));
        runner->executeCommand(
            fmt::format("mkdir -p {0}/var/lib/munge {0}/var/log/munge "
                        "{0}/etc/munge {0}/run/munge",
                m_stateless.chroot.string()));
        runner->executeCommand(fmt::format(
            "chown munge:munge {}/var/lib/munge", m_stateless.chroot.string()));
        runner->executeCommand(fmt::format(
            "chown munge:munge {}/var/log/munge", m_stateless.chroot.string()));
        runner->executeCommand(fmt::format(
            "chown munge:munge {}/etc/munge", m_stateless.chroot.string()));
        runner->executeCommand(fmt::format(
            "chown munge:munge {}/run/munge", m_stateless.chroot.string()));
    }

    for (const auto& script : customizations) {
        runner->run(script);
    };
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

    auto runner = cloyster::Singleton<IRunner>::get();
    for (const auto& command : commands) {
        runner->executeCommand(command);
    }
}

cloyster::services::XCAT::ImageInstallArgs
XCAT::getImageInstallArgs(ImageType imageType, NodeType nodeType)
{
    generateOSImageName(imageType, nodeType);
    generateOSImagePath(imageType, nodeType);
    LOG_ASSERT(!m_stateless.osimage.empty(), "Empty osimage name");
    return ImageInstallArgs {
        .imageName = m_stateless.osimage,
        .rootfs = m_stateless.chroot,
        .postinstall = "/install/custom/netboot/compute.postinstall",
        .pkglist = "/install/custom/netboot/compute.otherpkglist"
    };
}

/* This method will create an image for compute nodes, by default it will be a
 * stateless image with default services.
 */
void XCAT::createImage(ImageType imageType, NodeType nodeType, const std::vector<ScriptBuilder>& customizations)
{
    configureEL9();

    generateOSImageName(imageType, nodeType);

    const auto imageExists_ = imageExists(m_stateless.osimage);
    if (!imageExists_) {
        copycds(cluster()->getDiskImage().getPath());
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
        customizeImage(customizations);
        packimage();
    }
}

void XCAT::addNode(const Node& node)
{
    LOG_DEBUG("Adding node {} to xCAT", node.getHostname())

    std::string command = fmt::format(
        "mkdef -f -t node {} arch={} ip={} mac={} groups=compute,all "
        "netboot=xnba ",
        node.getHostname(),
        cloyster::utils::enums::toString(node.getOS().getArch()),
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

    cloyster::Singleton<IRunner>::get()->executeCommand(command);
}

void XCAT::addNodes()
{
    for (const auto& node : cluster()->getNodes()) {
        addNode(node);
    }

    auto runner = cloyster::Singleton<IRunner>::get();

    // TODO: Create separate functions
    runner->executeCommand("makehosts");
    runner->executeCommand("makedhcp -n");
    runner->executeCommand("makedns -n");
    runner->executeCommand("makegocons");
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
    auto runner = cloyster::Singleton<IRunner>::get();
    runner->executeCommand("rsetboot compute net");
}

void XCAT::resetNodes()
{
    auto runner = cloyster::Singleton<IRunner>::get();
    runner->executeCommand("rpower compute reset");
}

void XCAT::generateOSImageName(ImageType imageType, NodeType nodeType)
{
    std::string osimage = getOSImageDistroVersion();
    osimage += "-";

    switch (cluster()->getNodes()[0].getOS().getArch()) {
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
    chroot += getOSImageDistroVersion();
    chroot += "/";

    switch (cluster()->getNodes()[0].getOS().getArch()) {
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

std::vector<std::string> XCAT::getxCATOSImageRepos() const
{
    const auto osinfo = cluster()->getHeadnode().getOS();
    const auto repoManager = cloyster::Singleton<RepoManager>::get();
    std::vector<std::string> repos;
    const auto addReposFromFile = [&](const std::string& filename) {
        for (auto& repo : repoManager->repoFile(filename)) {
            if (repo->enabled()) {
                repos.emplace_back(repo->uri().value());
            }
        }
    };

    switch (osinfo.getDistro()) {
        case OS::Distro::RHEL:
            addReposFromFile("rhel.repo");
            break;
        case OS::Distro::OL:
            addReposFromFile("oracle.repo");
            break;
        case OS::Distro::Rocky:
            RockyLinux::shouldUseVault(osinfo) ?
                addReposFromFile("rocky-vault.repo") :
                addReposFromFile("rocky.repo");
            break;
        case OS::Distro::AlmaLinux:
            addReposFromFile("almalinux.repo");
            break;
    }

    addReposFromFile("epel.repo");
    addReposFromFile("OpenHPC.repo");

    return repos;
}

};
