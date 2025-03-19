/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/ofed.h>
#include <utility>

using cloyster::runCommand;

namespace {

auto docaRepoTemplate(std::string version, std::string distro)
{
    static constexpr std::string_view templ = R"( 
[doca] 
name=NVIDIA DOCA Repository - RHEL {1} 
baseurl=https://linux.mellanox.com/public/repo/doca/{0}/{1}/x86_64/ 
enabled=1 
gpgcheck=1 
gpgkey=https://linux.mellanox.com/public/repo/doca/{0}/GPG-KEY-Mellanox 
)"; 
    std::istringstream data(fmt::format(templ, version, distro)); 
    return data;
}

// Return distro to match repositories at
// https://linux.mellanox.com/public/repo/doca/latest/
std::string headnodeDistroName()
{
    using cloyster::models::Cluster;
    auto cluster = cloyster::Singleton<Cluster>::get();
    switch (cluster->getHeadnode().getOS().getDistro()) {
        // Assuming we'll be using the last distro version
        case cloyster::OS::Distro::RHEL:
            return "rhel9.5";
        case cloyster::OS::Distro::OL:
            return "ol9.4";
        case cloyster::OS::Distro::Rocky:
            return "rockylinux9.2";
        case cloyster::OS::Distro::AlmaLinux:
            return "alinux3.2";
        default:
            std::unreachable();
    };

    std::unreachable();
}

void installMellanoxDoca(const OFED& ofed)
{
}
};

void OFED::setKind(Kind kind) { m_kind = kind; }

OFED::Kind OFED::getKind() const { return m_kind; }

void OFED::install() const
{
    switch (m_kind) {
        case OFED::Kind::Inbox:
            runCommand("dnf -y groupinstall \"Infiniband Support\"");

            break;

        case OFED::Kind::Mellanox:
            {
                auto runner = cloyster::Singleton<cloyster::services::BaseRunner>::get();
                auto repoManager = cloyster::Singleton<cloyster::services::repos::RepoManager>::get();

                if (runner->executeCommand("modprobe mlx5_core") == 0) {
                    LOG_WARN("mlx5_core module loaded, skiping DOCA setup");
                    return;
                }

                auto repoData = docaRepoTemplate(getVersion(), headnodeDistroName());
                std::filesystem::path path = "/etc/yum.repos.d/mlx-doca.repo";

                // Install the repository and enable it
                cloyster::installFile(path, repoData);
                repoManager->install(path);
                repoManager->enable("doca");

                // Install the required packages
                runner->executeCommand("dnf makecache");
                runner->executeCommand("dnf install –y kernel kernel-devel doca-extra");

                // Run the Mellanox script, this generates an RPM at tmp
                assert(runner->executeCommand("/opt/mellanox/doca/tools/doca-kernel-support -k $(rpm -q --qf \"%{VERSION}-%{RELEASE}.%{ARCH}\n\" kernel-devel") == 0);

                // Install the (last) generated rpm
                runner->executeCommand("rpm -ivh $(find /tmp/DOCA.*/ -name '*.rpm' -printf \"%T@ %p\n\" | sort -nrk1 | tail -1 | awk '{print $2}')");

                runner->executeCommand("dnf makecache");
                runner->executeCommand("dnf install –y kernel kernel-devel doca-extra");
                if (runner->executeCommand("lsmod | grep mlx5_core") != 0) {
                    runner->executeCommand("modprobe mlx_core");
                }

            }
            break;

        case OFED::Kind::Oracle:
            throw std::logic_error("Oracle RDMA release is not yet supported");

            break;
    }
}
