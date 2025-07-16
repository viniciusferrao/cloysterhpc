#ifndef CLOYSTERHPC_ANSIBLE_ROLE_H_
#define CLOYSTERHPC_ANSIBLE_ROLE_H_

#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <cloysterhpc/models/os.h>

namespace cloyster::services::ansible::roles {

/**
 * @brief Represents an Ansible role and its associated metadata.
 *
 * This struct models a role defined in an Ansible playbook, including its
 * name, conditional execution, tags, and variables. It is used as input for
 * generating shell-based installation scripts tailored to each role.
 *
 * Members:
 * - `m_roleName`: The name of the role (e.g., "base", "audit").
 * - `m_tags`: A list of tags associated with the role (e.g., ["audit",
 * "security"]).
 * - `m_vars`: A key-value map of variables defined for the role.
 */
struct Role {
    using Variables = std::unordered_map<std::string, std::string>;

    std::string m_roleName;
    std::vector<std::string> m_tags;
    Variables m_vars;
    std::optional<std::function<bool(const models::OS&)>> m_when = std::nullopt;
};

/**
 * @brief Parses a role string in the format `<rolename>:var1=val1,var2=val2`
 *        and constructs a Role object.
 *
 * This function extracts the role name and its associated key-value variables
 * from a single input string. It validates the format and populates a
 * Role struct accordingly.
 *
 * @param input A string in the format `<rolename>:var1=val1,var2=val2`.
 * @return Role The constructed Role object with populated m_roleName and
 * m_vars.
 *
 * @throws std::invalid_argument if the input format is invalid or parsing
 * fails.
 *
 * @note If no variables are present (e.g., `base:`), m_vars will be empty.
 * @note `m_when` and `m_tags` fields in Role are not populated by this
 * function.
 *
 * @see Role
 */
Role parseRoleString(const std::string& input);

}

// Custom formatter for ansible::Role
template <> struct fmt::formatter<cloyster::services::ansible::roles::Role> {
    static constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const cloyster::services::ansible::roles::Role& role,
        FormatContext& ctx)
    {
        std::string result = fmt::format("Role: {}", role.m_roleName);

        if (!role.m_tags.empty()) {
            result += "\n  Tags:";
            for (const auto& tag : role.m_tags) {
                result += fmt::format(" {}", tag);
            }
        }
        if (!role.m_vars.empty()) {
            result += "\n  Vars:";
            for (const auto& [key, val] : role.m_vars) {
                result += fmt::format(" {}={}", key, val);
            }
        }

        return fmt::format_to(ctx.out(), "{}", result);
    }
};

#endif // ANSIBLE_ROLE_HPP
