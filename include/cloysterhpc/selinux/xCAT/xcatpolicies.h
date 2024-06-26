/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_XCATPOLICIES_H_
#define CLOYSTERHPC_XCATPOLICIES_H_

constexpr const char* xCATSELinuxPolicyFile =
#include "cloysterhpc/selinux/xCAT/xCAT-httpd-read-tftpdir.te"
    ;

constexpr const char* xCATgenimageSELinuxPolicyFile =
#include "cloysterhpc/selinux/xCAT/xCAT-genimage-ldconfig-rw-tmpdir.te"
    ;

constexpr const char* xCATrsyncSELinuxPolicyFile =
#include "cloysterhpc/selinux/xCAT/xCAT-rsync.te"
    ;

#endif // CLOYSTERHPC_XCATPOLICIES_H_
