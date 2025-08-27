#ifndef CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SLURM_H
#define CLOYSTERHPC_SERVICES_ANSIBLE_ROLES_SLURM_H

#include <cloysterhpc/services/ansible/role.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::ansible::roles::slurm {

void run(const Role& role);

};

#endif
