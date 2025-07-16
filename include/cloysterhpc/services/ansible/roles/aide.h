#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_AIDE_H_
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_AIDE_H_

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::aide {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo);

} // namespace cloyster::services::ansible::roles::aide

#endif // CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_AIDE_H_
