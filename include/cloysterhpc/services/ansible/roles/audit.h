#ifndef CLOYSTER_SERVICES_ANSIBLE_ROLE_AUDIT_H_
#define CLOYSTER_SERVICES_ANSIBLE_ROLE_AUDIT_H_

#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/services/ansible/role.h>

namespace cloyster::services::ansible::roles::audit {

ScriptBuilder installScript(
    const Role& role,
    const cloyster::models::OS& osinfo
);

};

#endif
