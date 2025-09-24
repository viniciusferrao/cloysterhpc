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
#include <cloysterhpc/utils/singleton.h>

namespace cloyster::services::ansible::roles {

/**
 * @brief Enumeration of all supported Ansible roles in the system.
 *
 * This enum provides identifiers for each available Ansible role that can be
 * executed. The values correspond to the role names and are used for
 * dispatching to the appropriate role implementation.
 */
enum class Roles : std::uint8_t {
    CHECK, ///< Pre install check/validation role
    REPOS, ///< Repository configuration role
    NETWORK, ///< Network configuration role
    SSHD, ///< SSH daemon configuration role
    OFED, ///< OpenFabrics Enterprise Distribution role
    DUMP, ///< System dump/crash role
    LOCALE, ///< System locale configuration role
    FIREWALL, ///< Firewall configuration role
    SELINUX, ///< SELinux configuration role
    NFS, ///< NFS server/client role
    QUEUESYSTEM, ///< Queue system role, SLURM + PBS logic
    SLURM, ///< SLURM role
    OHPC, ///< OpenHPC components role
    PROVISIONER, ///< Dispatches to XCAT or CONFLUENT roles
    XCAT, ///< xCAT management role
    CONFLUENT, ///< Confluent platform role
    BASE, ///< Base system configuration role
    AUDIT, ///< Audit system role
    AIDE, ///< Advanced Intrusion Detection Environment role
    FAIL2BAN, ///< Fail2ban security role
    TIMESYNC, ///< Time synchronization role
    SPACK, ///< Spack package manager role
};

/**
 * @brief Represents an Ansible role and its associated metadata.
 *
 * This struct models a role defined in an Ansible playbook, including its
 * name, conditional execution, tags, and variables. It is used as input for
 * generating shell-based installation scripts tailored to each role.
 */
class Role {
public:
    using Vars = std::unordered_map<std::string, std::string>;
    using When = std::function<bool(const models::OS&)>;
    using Tags = std::set<std::string>;

private:
    Roles m_role;
    std::string m_roleName;
    Tags m_tags;
    Vars m_vars;
    std::optional<When> m_when = std::nullopt;

public:
    Role(Roles role, auto&& tags, auto&& vars, auto&& when)
        : m_role(role)
        , m_roleName(utils::enums::toStringLower(role))
        , m_tags(std::forward<decltype(tags)>(tags))
        , m_vars(std::forward<decltype(vars)>(vars))
        , m_when(std::forward<decltype(when)>(when))
    {
    }

    auto role() const -> const auto& { return m_role; }
    auto roleName() const -> const auto& { return m_roleName; }
    auto vars() const -> const auto& { return m_vars; }
    auto tags() const -> const auto& { return m_tags; }
    auto when() const -> const auto& { return m_when; }
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
        std::string result = fmt::format("Role: {}", role.roleName());

        if (!role.tags().empty()) {
            result += "\n  Tags:";
            for (const auto& tag : role.tags()) {
                result += fmt::format(" {}", tag);
            }
        }
        if (!role.vars().empty()) {
            result += "\n  Vars:";
            for (const auto& [key, val] : role.vars()) {
                result += fmt::format(" {}={}", key, val);
            }
        }

        return fmt::format_to(ctx.out(), "{}", result);
    }
};

#endif // ANSIBLE_ROLE_HPP
