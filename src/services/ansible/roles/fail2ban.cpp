// src/services/ansible/roles/fail2ban.cpp

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/ansible/roles/fail2ban.h>
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
#include <string_view>

namespace cloyster::services::ansible::roles::fail2ban {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo)
{
    using namespace cloyster;
    ScriptBuilder builder(osinfo);

    LOG_ASSERT(role.m_roleName == "fail2ban",
        fmt::format("Expected fail2ban role, found {}", role.m_roleName));

    builder.addNewLine()
        .addCommand("# Install fail2ban package")
        .addPackage("fail2ban")
        .addNewLine()
        .addCommand("# Add fail2ban jail.local configuration")
        .addFileTemplate("/etc/fail2ban/jail.local", R"EOF(
[DEFAULT]
bantime  = 10m
findtime  = 10m
maxretry = 5
backend = systemd

[sshd]
enabled = true
)EOF")
        .addNewLine()
        .addCommand("# Enable and start fail2ban service")
        .enableService("fail2ban");

    return builder;
}

} // namespace cloyster::services::ansible::roles::fail2ban
