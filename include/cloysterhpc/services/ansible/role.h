#ifndef CLOYSTERHPC_ANSIBLE_ROLE_H_
#define CLOYSTERHPC_ANSIBLE_ROLE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <fmt/core.h>
#include <fmt/format.h>

namespace cloyster::services::ansible {

struct Role {
    using Variables = std::unordered_map<std::string, std::string>;

    std::string m_roleName;
    std::optional<std::string> m_whenCondition;
    std::vector<std::string> m_tags;
    Variables m_vars;
};

} // namespace ansible

// Custom formatter for ansible::Role
template <>
struct fmt::formatter<cloyster::services::ansible::Role> {
    static constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const cloyster::services::ansible::Role& role, FormatContext& ctx) {
        std::string result = fmt::format("Role: {}", role.m_roleName);

        if (role.m_whenCondition) {
            result += fmt::format("\n  When: {}", *role.m_whenCondition);
        }
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
