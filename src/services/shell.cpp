/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/ansible/roles/aide.h>
#include <cloysterhpc/services/ansible/roles/audit.h>
#include <cloysterhpc/services/ansible/roles/fail2ban.h>
#include <cloysterhpc/services/ansible/roles/spack.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>

#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <chrono>
#include <fmt/format.h>
#include <memory>

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/pbs.h>
#include <cloysterhpc/models/queuesystem.h>
#include <cloysterhpc/models/slurm.h>
#include <cloysterhpc/utils/singleton.h>

#include <cloysterhpc/dbus_client.h>
#include <ranges>

using cloyster::models::Cluster;
using cloyster::models::OS;
using cloyster::services::IOSService;
using cloyster::services::IRunner;

namespace cloyster::services {

Shell::Shell()
{
    // Initialize directory tree
    functions::createDirectory(std::string { installPath } + "/backup");
    functions::createDirectory(
        std::string { installPath } + "/conf/node/etc/auto.master.d");
}

/* This method is the entrypoint of shell based cluster install
 * The first session of the method will configure and install services on the
 * headnode. The last part will do provisioner related settings and image
 * creation for network booting
 */
void Shell::install()
{
    namespace ansible = ansible::roles;
    constexpr auto run = [](ansible::Roles role) {
        const auto osinfo = utils::singleton::os();
        ansible::run(role, osinfo);
    };

    run(ansible::Roles::DUMP);
    run(ansible::Roles::CHECK);
    run(ansible::Roles::REPOS);
    run(ansible::Roles::BASE);
    run(ansible::Roles::NETWORK);
    run(ansible::Roles::SSHD);
    run(ansible::Roles::SELINUX);
    run(ansible::Roles::LOCALE);
    run(ansible::Roles::TIMESYNC);
    run(ansible::Roles::FIREWALL);
    run(ansible::Roles::OFED);
    run(ansible::Roles::FAIL2BAN);
    run(ansible::Roles::AUDIT);
    run(ansible::Roles::AIDE);
    run(ansible::Roles::SPACK);
    run(ansible::Roles::NFS);
    run(ansible::Roles::OHPC);
    run(ansible::Roles::QUEUESYSTEM);
    run(ansible::Roles::PROVISIONER);
}

}
