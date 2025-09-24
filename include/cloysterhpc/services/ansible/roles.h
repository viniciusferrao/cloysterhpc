#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_

#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/osservice.h>

// All ansible roles implemented, keep it sorted
// in alphabetic order
#include <cloysterhpc/services/ansible/roles/aide.h>
#include <cloysterhpc/services/ansible/roles/audit.h>
#include <cloysterhpc/services/ansible/roles/base.h>
#include <cloysterhpc/services/ansible/roles/check.h>
#include <cloysterhpc/services/ansible/roles/confluent.h>
#include <cloysterhpc/services/ansible/roles/dump.h>
#include <cloysterhpc/services/ansible/roles/fail2ban.h>
#include <cloysterhpc/services/ansible/roles/firewall.h>
#include <cloysterhpc/services/ansible/roles/locale.h>
#include <cloysterhpc/services/ansible/roles/network.h>
#include <cloysterhpc/services/ansible/roles/nfs.h>
#include <cloysterhpc/services/ansible/roles/ofed.h>
#include <cloysterhpc/services/ansible/roles/ohpc.h>
#include <cloysterhpc/services/ansible/roles/ood.h>
#include <cloysterhpc/services/ansible/roles/provisioner.h>
#include <cloysterhpc/services/ansible/roles/queuesystem.h>
#include <cloysterhpc/services/ansible/roles/repos.h>
#include <cloysterhpc/services/ansible/roles/selinux.h>
#include <cloysterhpc/services/ansible/roles/slurm.h>
#include <cloysterhpc/services/ansible/roles/spack.h>
#include <cloysterhpc/services/ansible/roles/sshd.h>
#include <cloysterhpc/services/ansible/roles/timesync.h>
#include <cloysterhpc/services/ansible/roles/xcat.h>

#include <cloysterhpc/services/execution.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles {

/**
 * @brief Represents a callable unit of Ansible role logic.
 *
 * `RoleRunnable` is a type alias for a standard C++ function object
 * (`std::function`). It encapsulates the executable logic for a specific
 * Ansible role, making it possible to store and invoke different role
 * implementations in a uniform way.
 *
 * The function signature defined by `RoleRunnable` takes a single constant
 * reference to a `Role` object as its parameter and returns nothing (`void`).
 * This design ensures that all runnable role implementations share a common
 * interface, regardless of whether they are a simple function, a lambda
 * expression, or a function object.
 *
 * This type is a key component of the dispatcher pattern used by the
 * `getRunnable()` function, allowing it to return a generic callable that can
 * be executed later.
 */
using RoleRunnable = std::function<void(const Role& role)>;

/**
 * @brief Retrieves a runnable role function based on the role name.
 *
 * This function acts as a factory or dispatch mechanism, returning the
 * appropriate function pointer or functor (a `RoleRunnable`) to execute a
 * specific Ansible role's `run` or `installScript` method.
 *
 * It maps a given `role.m_roleName` string to a corresponding
 * function. For some roles, it directly returns a function pointer (e.g.,
 * `repos::run`), while for others (the `installScript` roles), it wraps the
 * script-generating function in a lambda to ensure it matches the
 * `RoleRunnable` signature.
 *
 * @param role The `Role` object containing the name of the role to get the
 * runnable for.
 * @param osinfo The `models::OS` object, providing operating system
 * information, which is passed to the script-generating functions.
 * @return A `RoleRunnable` functor or function pointer that can be invoked to
 * execute the logic for the specified role.
 * @throws std::invalid_argument Throws an exception if the `role.m_roleName`
 * does not match any of the known roles.
 */
RoleRunnable getRunnable(const Role& role, const models::OS& osinfo);

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
void run(Roles role, const models::OS& osinfo, Role::Vars&& vars = {},
    Role::Tags&& tags = {},
    std::optional<
        std::function<bool(const models::OS& osinfo)>>&& = std::nullopt);

/**
 * @brief Executor implementation for Ansible roles.
 *
 * This class provides the concrete implementation of the Execution interface
 * specifically for executing Ansible roles. It executes the roles in the order
 * they appear in --roles command line argument. If --roles the Shell execution
 * is used instead.
 */
class Executor final : public Execution {
public:
    void install() override;
};

}

#endif // CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_H_
