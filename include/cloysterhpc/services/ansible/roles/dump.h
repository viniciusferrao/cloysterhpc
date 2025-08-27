#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_DUMP_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_DUMP_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::dump {

void run(const Role& role);

}

#endif
