#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_CONFLUENT_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_CONFLUENT_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::confluent {

void run(const Role& role);

};

#endif
