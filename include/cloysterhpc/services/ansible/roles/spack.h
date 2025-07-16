#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SPACK_H_
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SPACK_H_

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::spack {

/**
 * @brief Generates the install script for the spack role.
 *
 * @param role The Ansible role data.
 * @param osinfo The operating system information.
 * @return ScriptBuilder with the generated commands.
 */
ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo);

} // namespace cloyster::services::ansible::roles::spack

#endif // CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SPACK_H_
