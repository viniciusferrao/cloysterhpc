/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/ofed.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <utility>

using cloyster::IRunner;

namespace {

auto docaRepoTemplate(std::string version, std::string distro, std::string arch)
{
    // @FIXME: 1 There is no mirror for mellanox now, I'm adding this
    //  so the user can support airgap and local mirrors if we wants.
    //  When we create the mirror the repository must follow the same
    //  format (as execpted from any mirror)
    // @FIXME: 2 all this logic to deal wil the mirrors should be elsewhere
    // @FIXME: 3 the upstream URL must be extracted to a configuration that
    //   the enduser can change at runtime
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    static constexpr std::string_view template_ = R"(
[doca]
name=NVIDIA DOCA Repository - RHEL {2}
baseurl={0}
enabled=1
gpgcheck=1
gpgkey={1}
)";

    // Assemble the urls for mirror an upstream
    const std::string repoName = "mellanox";
    const std::string path = "/public/repo/doca";
    const std::string mirrorUrl = fmt::format(
        "{}/{}/{}/{}/{}/{}", opts->mirrorBaseUrl, repoName, path, version, distro, arch);
    const std::string upstreamUrl = fmt::format(
        "https://linux.mellanox.com/{}/{}/{}/{}/", path, version, distro, arch);
    const std::string upstreamGPG = fmt::format(
        "https://linux.mellanox.com/public/repo/doca/{}/{}/{}/GPG-KEY-Mellanox.pub",
         version, distro, arch);
    const std::string mirrorGPG = fmt::format(
        "{}/{}/{}/{}/{}/{}/GPG-KEY-Mellanox.pub",
        opts->mirrorBaseUrl, repoName, path, version, distro, arch);

    if (opts->disableMirrors) {
        auto upstreamStatus = cloyster::utils::getHttpStatus(upstreamUrl);
        if (upstreamStatus != "200") {
            throw std::runtime_error(
                fmt::format("BUG: Upstream {} offline. This may be a BUG, to "
                            "mitigate create the file "
                            "/etc/yum.repos.d/mlx-doca.repo and try again",
                            upstreamUrl));
        }
        std::istringstream data(fmt::format(template_, upstreamUrl, upstreamGPG, version));
        return data;
    } else {
        const auto mirrorStatus = cloyster::utils::getHttpStatus(mirrorUrl);
        if (mirrorStatus == "200") {
            std::istringstream data(fmt::format(template_, mirrorUrl, mirrorGPG, version));
            return data;
        } else {
            // The mirror is down, falling back to upstream
            LOG_WARN("Mirror {} unavailable HTTP {}, falling back to upstream {}",
                     mirrorUrl, mirrorStatus, upstreamUrl);

            auto upstreamStatus = cloyster::utils::getHttpStatus(upstreamUrl);
            if (upstreamStatus != "200") {
                throw std::runtime_error(
                    fmt::format("BUG: Upstream {} offline. This may be a BUG, to "
                                "mitigate create the file "
                                "/etc/yum.repos.d/mlx-doca.repo and try again",
                                upstreamUrl));
            }
            std::istringstream data(fmt::format(template_, upstreamUrl, upstreamGPG, version));
            return data;
        }
    }
}

};

void OFED::setKind(Kind kind) { m_kind = kind; }

OFED::Kind OFED::getKind() const { return m_kind; }

bool OFED::installed() const
{
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->shouldForce("infiniband-install")) {
        return false;
    }

    // Return false so the installation runs on dry run
    if (opts->dryRun) {
        return false;
    }

    auto runner = cloyster::Singleton<IRunner>::get();
    switch (m_kind) {
        case OFED::Kind::Mellanox:
            return runner->executeCommand("rpm -q doca-ofed") == 0;
        case OFED::Kind::Inbox:
            return runner->executeCommand(
                       "dnf group info \"Infiniband Support\"")
                == 0;
        case OFED::Kind::Oracle:
            throw std::logic_error("Not implemented");
    }

    std::unreachable();
}

void OFED::install() const
{
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();

    if (opts->dryRun) {
        LOG_WARN("Dry-Run: Skiping OFED installation");
        return;
    }

    // Idempotency check
    if (installed()) {
        LOG_WARN("Inifiniband already installed, skipping, use `--force "
                 "infiniband-install` to force");
        return;
    }


    switch (m_kind) {
        case OFED::Kind::Inbox:
            cloyster::Singleton<cloyster::services::IOSService>::get()
                ->groupInstall("Infiniband Support");
            break;

        case OFED::Kind::Mellanox: {
            auto cluster
                = cloyster::Singleton<cloyster::models::Cluster>::get();
            auto hnOs = cluster->getHeadnode().getOS();
            auto runner
                = cloyster::Singleton<cloyster::services::IRunner>::get();
            auto repoManager = cloyster::Singleton<
                cloyster::services::repos::RepoManager>::get();
            auto osService
                = cloyster::Singleton<cloyster::services::IOSService>::get();
            // distroName denotes a folder in the remote repository:
            // https://linux.mellanox.com/public/repo/doca/latest/. All
            // distributions except Oracle Linux use rhelX.Y folder. Oracle
            // Linux does not uses rhelX.Y package because it uses a distinct
            // kernel
            auto distroName = fmt::format("{}{}",
                (hnOs.getDistro() == cloyster::OS::Distro::OL ? "ol" : "rhel"),
                hnOs.getVersion());

            auto repoData = docaRepoTemplate(getVersion(), distroName,
                cloyster::utils::enums::toString(hnOs.getArch()));
            std::filesystem::path path = "/etc/yum.repos.d/mlx-doca.repo";

            // Install the repository and enable it
            cloyster::installFile(path, repoData);
            repoManager->install(path);
            repoManager->enable("doca");

            // Install the required packages
            runner->checkCommand("dnf makecache --repo=doca");
            runner->checkCommand(
                "dnf -y install kernel kernel-devel doca-extra");

            if (osService->getKernelRunning()
                != osService->getKernelInstalled()) {
                LOG_WARN("New kernel installed! Rebooting after the "
                         "installation finishes is advised!");
            }

            LOG_INFO("Compiling OFED DOCA drivers, this may take a while, use "
                     "`--skip compile-doca-driver` to skip");
            // Run the Mellanox script, this generates an RPM at tmp.
            //
            // Use the kernel-devel version instead of the booted kernel
            // version, this is to handle the case where a new kernel is
            // installed but no reboot was done yet. After compiling the
            // drivers the headnode should be rebooted to reload the new kernel.
            // The driver may support weak updates modules and load without
            // need for reboot.
            if (!opts->shouldSkip("compile-doca-driver")) {
                runner->checkCommand(
                    "bash -c \"/opt/mellanox/doca/tools/doca-kernel-support -k "
                    "$(rpm -q --qf \"%{VERSION}-%{RELEASE}.%{ARCH}\n\" "
                    "kernel-devel)\"");
            }

            // Get the last rpm in /tmp/DOCA*/ folder
            // On dry-run the below command will not run so we
            // cannot get the output of it
            auto rpm = runner->checkOutput(
                "bash -c \"find /tmp/DOCA*/ -name '*.rpm' -printf '%T@ %p\n' | "
                "sort -nk1 | tail -1 | awk '{print $2}'\"");
            assert(rpm.size() > 0); // at last one line

            // Install the (last) generated rpm
            runner->executeCommand(fmt::format("dnf install -y {}", rpm[0]));

            runner->checkCommand(R"(dnf makecache --repo=doca*)");
            runner->checkCommand("dnf install -y doca-ofed mlnx-fw-updater");
            runner->executeCommand("systemctl restart openibd");
        } break;

        case OFED::Kind::Oracle:
            throw std::logic_error("Oracle RDMA release is not yet supported");

            break;
    }
}
