#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/ansible/roles/timesync.h>
#include <cloysterhpc/services/log.h>
#include <fmt/core.h>
#include <string_view>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::services::ansible::roles::timesync {

ScriptBuilder installScript(
    const Role& role,
    const cloyster::models::OS& osinfo
) {
    using namespace cloyster;
    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.m_roleName == "timesync",
               fmt::format("Expected timesync role, found {}", role.m_roleName));

    builder
        .addNewLine()
        .addCommand("# Install and configure chrony for time synchronization")
        .addPackage("chrony")
        .enableService("chronyd");

    return builder;
}

} // namespace cloyster::services::ansible::roles::timesync
