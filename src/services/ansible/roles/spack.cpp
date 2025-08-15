#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/ansible/roles/spack.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/scriptbuilder.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>
#include <string_view>

namespace cloyster::services::ansible::roles::spack {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo)
{
    using namespace cloyster;
    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.m_roleName == "spack",
        fmt::format("Expected spack role, found {}", role.m_roleName));

    builder.addNewLine()
        .addCommand("# Exit early if spack is already installed")
        .addCommand("test -d /opt/spack/.git && exit 0")
        .addCommand("# Install dependencies for Spack")
        .addPackage("git")
        .addNewLine()
        .addCommand("# Clone Spack repository")
        .addCommand("git clone https://github.com/spack/spack.git /opt/spack")
        .addCommand("chown -R root:root /opt/spack")
        .addCommand("chmod -R 755 /opt/spack");

    return builder;
}

} // namespace cloyster::services::ansible::roles::spack
