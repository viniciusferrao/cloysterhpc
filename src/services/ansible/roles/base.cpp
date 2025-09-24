// src/services/ansible/roles/base.cpp

#include <set>

#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/utils/string.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>
namespace cloyster::services::ansible::roles::base {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo)
{
    using namespace cloyster;
    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.roleName() == "base",
        fmt::format("Expected base role, found {}", role.roleName()));

    builder.addNewLine().addCommand("# Install EPEL repositories if needed");

    switch (osinfo.getDistro()) {
        case models::OS::Distro::RHEL:
        case models::OS::Distro::Rocky:
        case models::OS::Distro::AlmaLinux:
            LOG_DEBUG("Running base role");
            break;

        case models::OS::Distro::OL:
            // @FIXME: This breaks the RepoManager logic. Package installing
            //   repository files at /etc/yum.repos.d/, may install repositories
            //   using metalink or mirrorlist, which triggers a bug in
            //   RepoManager when the xCAT image is being generated. The
            //   RepoManager only supports baseurl for now, metalink and mirror
            //   lists trigger a bad optional access during runtime.
            switch (osinfo.getPlatform()) {
                case models::OS::Platform::el8:
                    builder.addPackage("oracle-epel-release-el8");
                    break;
                case models::OS::Platform::el9:
                    builder.addPackage("oracle-epel-release-el9");
                    break;
                default:
                    builder.addCommand("# Unsupported Oracle Linux platform");
                    break;
            }
            break;

        default:
            builder.addCommand("# Unsupported distribution");
            break;
    }

    builder.addNewLine().addCommand("# Install general base packages");

    // "python3-dnf-plugin-versionlock" is conflicting with dnf-plugins-core
    // during the first install
    // TODO: CFL initscripts is only required by xCAT
    std::set<std::string> allPackages = {
        "wget",
        "curl",
        "dnf-plugins-core",
        "chkconfig",
        "initscripts", // @FIXME: This is only required if the provisioner is
                       // xCAT
        "jq",
        "tar",
    };
    if (const auto iter = role.vars().find("base_packages");
        iter != role.vars().end()) {
        for (const auto& pkg :
            cloyster::utils::string::split(iter->second, " ")) {
            allPackages.emplace(pkg);
        }
    }
    builder.addPackages(allPackages);

    // Configure timezone
    const auto& cluster = cloyster::Singleton<models::Cluster>::get();
    builder.addCommand(
        "timedatectl set-timezone {}", cluster->getTimezone().getTimezone());

    return builder;
}

} // namespace
