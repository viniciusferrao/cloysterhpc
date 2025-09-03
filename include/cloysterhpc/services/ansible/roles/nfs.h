#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_NFS_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_NFS_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::nfs {

ScriptBuilder installScript(const Role& role, const OS& osinfo);

}

#endif
