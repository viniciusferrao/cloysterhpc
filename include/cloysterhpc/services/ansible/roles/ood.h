#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OOD_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OOD_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::ood {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo);

};

#endif
