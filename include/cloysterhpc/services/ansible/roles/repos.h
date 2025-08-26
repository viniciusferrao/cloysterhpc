#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_REPOS_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_REPOS_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::repos {

void run(const Role& role);

}

#endif
