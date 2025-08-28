#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/utils/string.h>
#include <cloysterhpc/utils/optional.h>

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
    Role::Vars vars;
    Role::Tags tags;
    Role::When when;

    const auto colonPos = input.find(':');
    const bool hasVars = colonPos != std::string::npos;

    // If no colon, assume input is only role name with no variables
    std::string_view roleName = std::string_view(input).substr(
        0, !hasVars ? input.size() : colonPos);
    auto roleEnum = utils::enums::ofStringExc<Roles>(
        roleName, utils::enums::Case::Insensitive);

    if (hasVars) {
        // Otherwise, parse role name and optional variables
        std::string varsPart = input.substr(colonPos + 1);

        std::stringstream stream(varsPart);
        std::string pair;

        while (std::getline(stream, pair, ',')) {
            auto eqPos = pair.find('=');
            if (eqPos == std::string::npos || eqPos == 0
                || eqPos == pair.size() - 1) {
                throw std::invalid_argument(
                    "Each variable must be in format key=value");
            }

            std::string key = pair.substr(0, eqPos);
            std::string value = pair.substr(eqPos + 1);

            vars[key] = value;
        }
    }

    return Role{roleEnum, tags, vars, when};
}

TEST_CASE("ansible::Role formatter produces correct output")
{
    ansible::roles::Role role(Roles::AUDIT,
        Role::Tags{ "security", "compliance" },
        Role::Vars{ { "auditd_enabled", "true" }, { "log_level", "debug" } },
        std::nullopt
        );

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
