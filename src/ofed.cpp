/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/ofed.h>
#include <utility>

using cloyster::IRunner;
using cloyster::runCommand;

namespace {

auto docaRepoTemplate(std::string version, std::string distro, std::string arch)
{
    static constexpr std::string_view templ = R"(
[doca]
name=NVIDIA DOCA Repository - RHEL {1}
baseurl=https://linux.mellanox.com/public/repo/doca/{0}/{1}/{2}/
enabled=1
gpgcheck=1
gpgkey=https://linux.mellanox.com/public/repo/doca/{0}/{1}/{2}/GPG-KEY-Mellanox.pub
)";
    std::istringstream data(fmt::format(templ, version, distro, arch));
    return data;
}

};

void OFED::setKind(Kind kind) { m_kind = kind; }

OFED::Kind OFED::getKind() const { return m_kind; }

bool OFED::installed() const
{
    if (cloyster::shouldForce("infiniband-install")) {
        return false;
    }

    // Return false so the installation runs on dry run
    if (cloyster::dryRun) {
        return false;
    }

    auto runner = cloyster::Singleton<IRunner>::get();
    switch (m_kind) {
        case OFED::Kind::Mellanox:
            return runner->executeCommand("rpm -q doca-ofed") == 0;
        case OFED::Kind::Inbox:
            return runner->executeCommand("dnf group info \"Infiniband Support\"") == 0;
        case OFED::Kind::Oracle:
            throw std::logic_error("Not implemented");
    }

    std::unreachable();
}

void OFED::install() const
{
    // Idempotency check
    if (installed()) {
        LOG_WARN("Inifiniband already installed, skipping, use `--force infiniband-install` to force");
        return;
    }

    switch (m_kind) {
        case OFED::Kind::Inbox:
            runCommand("dnf -y groupinstall \"Infiniband Support\"");
            break;

        case OFED::Kind::Mellanox:
            {
                auto cluster = cloyster::Singleton<cloyster::models::Cluster>::get();
                auto hnOs = cluster->getHeadnode().getOS();
                auto runner = cloyster::Singleton<cloyster::services::IRunner>::get();
                auto repoManager = cloyster::Singleton<cloyster::services::repos::RepoManager>::get();
                auto osService = cloyster::Singleton<cloyster::services::IOSService>::get();
                // distroName denotes a folder in the remote repository:
                // https://linux.mellanox.com/public/repo/doca/latest/. All
                // distributions except Oracle Linux use rhelX.Y folder. Oracle
                // Linux does not uses rhelX.Y package because it uses a distinct
                // kernel
                auto distroName = fmt::format(
                    "{}{}",
                    (hnOs.getDistro() == cloyster::OS::Distro::OL ? "ol" : "rhel"),
                    hnOs.getVersion());

                auto repoData = docaRepoTemplate(
                    getVersion(),
                    distroName,
                    cloyster::utils::enumToString(hnOs.getArch()));
                std::filesystem::path path = "/etc/yum.repos.d/mlx-doca.repo";

                // Install the repository and enable it
                cloyster::installFile(path, repoData);
                repoManager->install(path);
                repoManager->enable("doca");

                // Install the required packages
                runner->checkCommand("dnf makecache --repo=doca");
                runner->checkCommand("dnf -y install kernel kernel-devel doca-extra");

                if (osService->getKernelRunning() != osService->getKernelInstalled()) {
                    LOG_WARN("New kernel installed! Rebooting after the installation finishes is advised!");
                }

                LOG_INFO("Compiling OFED DOCA drivers, this may take a while, use `--skip compile-doca-driver` to skip");
                // Run the Mellanox script, this generates an RPM at tmp.
                //
                // Use the kernel-devel version instead of the booted kernel
                // version, this is to handle the case where a new kernel is
                // installed but no reboot was done yet. After compiling the
                // drivers the headnode should be rebooted to reload the new kernel.
                // The driver may support weak updates modules and load without
                // need for reboot.
                if (!cloyster::shouldSkip("compile-doca-driver")) {
                    runner->checkCommand(
                        "bash -c \"/opt/mellanox/doca/tools/doca-kernel-support -k "
                        "$(rpm -q --qf \"%{VERSION}-%{RELEASE}.%{ARCH}\n\" kernel-devel)\"");
                }

                // Get the last rpm in /tmp/DOCA*/ folder
                auto rpm = runner->checkOutput("bash -c \"find /tmp/DOCA*/ -name '*.rpm' -printf '%T@ %p\n' | sort -nk1 | tail -1 | awk '{print $2}'\"");
                assert(rpm.size() > 0); // at last one line

                // Install the (last) generated rpm
                runner->executeCommand(fmt::format("dnf install -y {}", rpm[0]));

                runner->checkCommand(R"(dnf makecache --repo=doca*)");
                runner->checkCommand("dnf install -y doca-ofed mlnx-fw-updater");
                runner->checkCommand("systemctl restart openibd");
            }
            break;

        case OFED::Kind::Oracle:
            throw std::logic_error("Oracle RDMA release is not yet supported");

            break;
    }
}
