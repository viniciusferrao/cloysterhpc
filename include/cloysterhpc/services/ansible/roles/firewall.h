#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_FIREWALL_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_FIREWALL_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::firewall {

void run(const Role& role);

};

#endif
