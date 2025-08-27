#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_PROVISIONER_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_PROVISIONER_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::provisioner {

void run(const Role& role);

};

#endif
