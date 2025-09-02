#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SSHD_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SSHD_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::sshd {

void run(const Role& role);
 

};

#endif
