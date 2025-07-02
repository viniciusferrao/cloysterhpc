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


namespace cloyster::services::ansible {

TEST_CASE("ansible::Role formatter produces correct output") {
    ansible::Role role{
        .m_roleName = "audit",
        .m_whenCondition = "ansible_os_family == 'RedHat'",
        .m_tags = {"security", "compliance"},
        .m_vars = {
            {"auditd_enabled", "true"},
            {"log_level", "debug"}
        }
    };

    std::string expected =
        "Role: audit\n"
        "  When: ansible_os_family == 'RedHat'\n"
        "  Tags: security compliance\n"
        "  Vars: auditd_enabled=true log_level=debug";

    std::string actual = fmt::format("{}", role);

    // Note: Since map iteration is unordered, we match parts instead of exact string
    CHECK(actual.find("Role: audit") != std::string::npos);
    CHECK(actual.find("When: ansible_os_family == 'RedHat'") != std::string::npos);
    CHECK(actual.find("Tags: security compliance") != std::string::npos);
    CHECK((actual.find("auditd_enabled=true") != std::string::npos));
    CHECK((actual.find("log_level=debug") != std::string::npos));
}

} // namespace 

