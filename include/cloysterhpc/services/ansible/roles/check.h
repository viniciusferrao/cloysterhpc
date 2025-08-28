#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_CHECK_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_CHECK_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::check {

void run(const Role& role);

};

#endif
