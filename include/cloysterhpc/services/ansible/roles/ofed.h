#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OFED_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_OFED_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::ofed {

void run(const Role& role);

}

#endif
