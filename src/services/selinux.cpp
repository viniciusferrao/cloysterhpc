/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/selinux/selinux.h"
#include "cloysterhpc/functions.h"
#include "cloysterhpc/services/log.h"

void SELinux::configurexCATPolicies()
{
    LOG_TRACE("Configuring xCAT SELinux policies")
    configurexCATPolicyFile();
    configurexCATgenimagePolicyFile();
    configurexCATrsyncPolicyFile();
}

void SELinux::configurexCATPolicyFile()
{
    // Create SELinux policy file for xCAT
    const std::string file = "/root/xCAT-httpd-read-tftpdir.te";
    cloyster::addStringToFile(file, xCATSELinuxPolicyFile);
    cloyster::setFilePermissions(file);

    // Install SELinux policy file for xCAT
    std::string combinedCommands
        = "checkmodule -M -m -o /root/xCAT-httpd-read-tftpdir.mod "
          "/root/xCAT-httpd-read-tftpdir.te && "
          "semodule_package -o /root/xCAT-httpd-read-tftpdir.pp -m "
          "/root/xCAT-httpd-read-tftpdir.mod && "
          "semodule -i /root/xCAT-httpd-read-tftpdir.pp && "
          "semanage fcontext -a -t httpd_sys_content_t '/install(/.*)?' && "
          "restorecon -R /install && "
          "rm -rf /root/xCAT-httpd-read-tftpdir*";

    cloyster::runCommand(combinedCommands);
}

void SELinux::configurexCATgenimagePolicyFile()
{
    // Create SELinux genimage policy file for xCAT
    const std::string file = "/root/xCAT-genimage-ldconfig-rw-tmpdir.te";
    cloyster::addStringToFile(file, xCATgenimageSELinuxPolicyFile);
    cloyster::setFilePermissions(file);

    // Install SELinux genimage policy file for xCAT
    std::string combinedCommands
        = "checkmodule -M -m -o /root/xCAT-genimage-ldconfig-rw-tmpdir.mod "
          "/root/xCAT-genimage-ldconfig-rw-tmpdir.te && "
          "semodule_package -o /root/xCAT-genimage-ldconfig-rw-tmpdir.pp -m "
          "/root/xCAT-genimage-ldconfig-rw-tmpdir.mod && "
          "semodule -i /root/xCAT-genimage-ldconfig-rw-tmpdir.pp && "
          "rm -rf /root/xCAT-genimage-ldconfig*";

    cloyster::runCommand(combinedCommands);
}

void SELinux::configurexCATrsyncPolicyFile()
{
    // Create SELinux rsync policy file for xCAT
    const std::string file = "/root/xCAT-rsync.te";
    cloyster::addStringToFile(file, xCATrsyncSELinuxPolicyFile);
    cloyster::setFilePermissions(file);

    // Install SELinux rsync policy file for xCAT
    std::string combinedCommands
        = "checkmodule -M -m -o /root/xCAT-rsync.mod /root/xCAT-rsync.te && "
          "semodule_package -o /root/xCAT-rsync.pp -m /root/xCAT-rsync.mod && "
          "semodule -i /root/xCAT-rsync.pp && "
          "rm -rf /root/xCAT-genimage-ldconfig*";

    cloyster::runCommand(combinedCommands);
}

void SELinux::configureProvisioner(Cluster::Provisioner provisioner)
{
    switch (provisioner) {
        case Cluster::Provisioner::xCAT:
            configurexCATPolicies();
            break;
    }
}
