#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/log.h>
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
    const Role& role,
    const cloyster::models::OS& osinfo
) {
    using namespace cloyster;
    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.m_roleName == "base",
               fmt::format("Expected base role, found {}", role.m_roleName));

    builder
        .addNewLine()
        .addCommand("# Install EPEL repositories if needed");

    switch (osinfo.getDistro()) {
        case models::OS::Distro::RHEL:
        case models::OS::Distro::Rocky:
        case models::OS::Distro::AlmaLinux:
            builder.addPackage("epel-release");
            break;

        case models::OS::Distro::OL:
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

    builder
        .addNewLine()
        .addCommand("# Install general base packages");

    if (const auto iter = role.m_vars.find("base_packages"); iter != role.m_vars.end()) {
        for (const auto& pkg : cloyster::utils::string::split(iter->second, " ")) {
            builder.addPackage(pkg);
        }
    }

    return builder;
}

} // namespace
