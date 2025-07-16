#include <cloysterhpc/services/ansible/roles/aide.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::aide {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo)
{
    using namespace cloyster;

    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.m_roleName == "aide",
        fmt::format("Expected aide role, found {}", role.m_roleName));

    builder.addNewLine()
        .addCommand("# Install AIDE package")
        .addPackage("aide")
        .addNewLine()
        .addCommand("# Skip if aide database exists")
        .addCommand("test -f /var/lib/aide/aide.db.gz && exit 0")
        .addCommand("# Initialize AIDE database")
        .addCommand("aide --init")
        .addCommand("mv /var/lib/aide/aide.db.new.gz /var/lib/aide/aide.db.gz");

    return builder;
}

} // namespace cloyster::services::ansible::roles::aide
