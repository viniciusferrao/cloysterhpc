#include <cloysterhpc/services/ansible/roles/xcat.h>
#include <cloysterhpc/services/xcat.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/NFS.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {
using namespace cloyster::utils::singleton;
}

namespace cloyster::services::ansible::roles::xcat {

void run(const Role& role)
{
    const auto repoManager = ::repos();
    const auto opts = ::options();
    const auto osinfo = ::os();
    std::unique_ptr<XCAT> provisioner;
    switch (::cluster()->getProvisioner()) {
        case Cluster::Provisioner::xCAT:
            repoManager->enable("xcat-core");
            repoManager->enable("xcat-dep");
            provisioner = std::make_unique<XCAT>();
            break;
    }

    const auto& provisionerName { cloyster::utils::enums::toString(
        ::cluster()->getProvisioner()) };
    LOG_INFO("Setting up compute node images... This may take a while")

    LOG_INFO("[{}] Installing provisioner packages", provisionerName)
    provisioner->installPackages();

    // TODO: CFL nfsInstallScript depends on provisioner here, double check
    // NFS requires /install and /tftpboot folders
    // ::runner()->run(nfsInstallScript);

    LOG_INFO("[{}] Patching the provisioner", provisionerName)
    provisioner->patchInstall();

    LOG_INFO("[{}] Setting up the provisioner", provisionerName)
    provisioner->setup();
    const auto imageType = XCAT::ImageType::Netboot;
    const auto nodeType = XCAT::NodeType::Compute;

    opts->maybeStopAfterStep("provisioner-setup");
    const auto imageInstallArgs
        = provisioner->getImageInstallArgs(imageType, nodeType);

    NFS networkFileSystem = NFS("pub", "/opt/ohpc",
        cluster()
            ->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getAddress(),
        "ro,no_subtree_check");
    // TODO: CFL NFS script is coupled to XCAT, generalize it
    const auto nfsInstallScript
        = networkFileSystem.installScript(cluster()->getHeadnode().getOS());
    // Customizations to the image
    const auto nfsImageInstallScript
        = networkFileSystem.imageInstallScript(osinfo, imageInstallArgs);

    // Image role
    LOG_INFO("[{}] Creating node images", provisionerName);
    provisioner->createImage(imageType, nodeType,
        { // Customizations to the image
            nfsImageInstallScript });
    opts->maybeStopAfterStep("provisioner-create-image");

    // nodes role
    LOG_INFO("[{}] Adding compute nodes", provisionerName)
    provisioner->addNodes();

    LOG_INFO("[{}] Setting up image on nodes", provisionerName)
    provisioner->setNodesImage();

    LOG_INFO("[{}] Setting up boot settings via IPMI, if available",
        provisionerName);
    provisioner->setNodesBoot();
    provisioner->resetNodes();

    // Fix slurmctld: error: Check for out of sync clocks
    LOG_INFO("Synchronizing clocks");
    osservice()->restartService("chronyd");
}

}
