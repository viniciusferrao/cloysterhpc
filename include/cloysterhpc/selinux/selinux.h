/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SELINUX_H_
#define CLOYSTERHPC_SELINUX_H_

#include "cloysterhpc/cluster.h"
#include <cloysterhpc/selinux/xCAT/xcatpolicies.h>

class SELinux {
private:
    void configurexCATPolicies();
    void configurexCATPolicyFile();
    void configurexCATgenimagePolicyFile();
    void configurexCATrsyncPolicyFile();
public:
    void configureProvisioner(Cluster::Provisioner provisioner);
};

#endif // CLOYSTERHPC_SELINUX_H_
