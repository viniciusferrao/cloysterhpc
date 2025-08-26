#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_XCAT_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_XCAT_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::xcat {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo);

};

#endif
