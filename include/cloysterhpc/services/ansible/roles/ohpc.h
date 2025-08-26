#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OHPC_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OHPC_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::ohpc {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo);

};

#endif
