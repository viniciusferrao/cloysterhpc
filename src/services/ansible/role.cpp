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

namespace cloyster::services::ansible::roles {

Role parseRoleString(const std::string& input)
{
    Role role;

    auto colonPos = input.find(':');

    // If no colon, assume input is only role name with no variables
    if (colonPos == std::string::npos) {
        role.m_roleName = input;
        return role;
    }

    // Otherwise, parse role name and optional variables
    role.m_roleName = input.substr(0, colonPos);
    std::string varsPart = input.substr(colonPos + 1);

    if (varsPart.empty()) {
        return role; // no variables provided
    }

    std::stringstream ss(varsPart);
    std::string pair;

    while (std::getline(ss, pair, ',')) {
        auto eqPos = pair.find('=');
        if (eqPos == std::string::npos || eqPos == 0
            || eqPos == pair.size() - 1) {
            throw std::invalid_argument(
                "Each variable must be in format key=value");
        }

        std::string key = pair.substr(0, eqPos);
        std::string value = pair.substr(eqPos + 1);

        role.m_vars[key] = value;
    }

    return role;
}

TEST_CASE("ansible::Role formatter produces correct output")
{
    ansible::roles::Role role { .m_roleName = "audit",
        .m_tags = { "security", "compliance" },
        .m_vars = { { "auditd_enabled", "true" }, { "log_level", "debug" } } };

    std::string expected = "Role: audit\n"
                           "  When: ansible_os_family == 'RedHat'\n"
                           "  Tags: security compliance\n"
                           "  Vars: auditd_enabled=true log_level=debug";

    std::string actual = fmt::format("{}", role);

    // Note: Since map iteration is unordered, we match parts instead of exact
    // string
    CHECK(actual.find("Role: audit") != std::string::npos);
    CHECK(actual.find("Tags: security compliance") != std::string::npos);
    CHECK((actual.find("auditd_enabled=true") != std::string::npos));
    CHECK((actual.find("log_level=debug") != std::string::npos));
}

} // namespace
