#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/ansible/roles/aide.h>
#include <cloysterhpc/services/ansible/roles/audit.h>
#include <cloysterhpc/services/ansible/roles/base.h>
#include <cloysterhpc/services/ansible/roles/fail2ban.h>
#include <cloysterhpc/services/ansible/roles/spack.h>
#include <cloysterhpc/services/ansible/roles/timesync.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles {

/**
 * @brief Dispatches the installation script builder for the given Ansible role.
 *
 * This function takes a Role object and uses its name to select the appropriate
 * role-specific installation script function. It constructs a ScriptBuilder
 * populated with the shell commands required to provision the system
 * for that role.
 *
 * Supported roles include:
 * - base
 * - audit
 * - aide
 * - fail2ban
 * - timesync
 * - spack
 *
 * @param role The Ansible Role to process. Must have a valid `m_roleName`.
 * @param osinfo The OS metadata to determine compatibility and
 * platform-specific behavior.
 * @return ScriptBuilder A builder object containing the installation script for
 * the given role.
 *
 * @throws std::invalid_argument if the role name is not recognized.
 */
ScriptBuilder installScript(const Role& role, const models::OS& osinfo);

/**
 * @brief Executes the installation script for a given Ansible role if the
 * condition matches.
 *
 * This function evaluates the optional `when` condition of the role against the
 * provided operating system information. If the condition is either not
 * specified or returns true, it retrieves the system-wide script runner and
 * prepares the installation script for execution.
 *
 * @param role The Ansible role containing installation logic, variables, and
 * optional condition.
 * @param osinfo The operating system information used to evaluate the role
 * condition.
 */
void run(const Role& role, const models::OS& osinfo);

/**
 * @brief A convenient overload for calling roles by name
 *
 * This function evaluates the optional `when` condition of the role against the
 * provided operating system information. If the condition is either not
 * specified or returns true, it retrieves the system-wide script runner and
 * prepares the installation script for execution.
 *
 * @param role The Ansible role containing installation logic, variables, and
 * optional condition.
 * @param osinfo The operating system information used to evaluate the role
 * condition.
 */
void run(std::string_view roleName, const models::OS& osinfo,
    std::unordered_map<std::string, std::string>&& vars = {},
    std::optional<
        std::function<bool(const models::OS& osinfo)>>&& = std::nullopt);

}

#endif // CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_
