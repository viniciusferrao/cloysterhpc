#include <cloysterhpc/services/ansible/roles/selinux.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {
using namespace cloyster::utils::singleton;

void disableSELinux()
{
    ::runner()->executeCommand("setenforce 0");

    const auto filename = CHROOT "/etc/sysconfig/selinux";

    cloyster::functions::backupFile(filename);
    cloyster::functions::changeValueInConfigurationFile(filename, "SELINUX", "disabled");

    LOG_WARN("SELinux has been disabled")

}
void configureSELinuxMode()
{
    LOG_INFO("Setting up SELinux")

    switch (cluster()->getSELinux()) {
        case cloyster::models::Cluster::SELinuxMode::Permissive:
            ::runner()->executeCommand("setenforce 0");
            /* Permissive mode */
            break;

        case cloyster::models::Cluster::SELinuxMode::Enforcing:
            /* Enforcing mode */
            ::runner()->executeCommand("setenforce 1");
            break;

        case cloyster::models::Cluster::SELinuxMode::Disabled:
            disableSELinux();
            break;
    }
}

}

namespace cloyster::services::ansible::roles::selinux {

void run(const Role& role)
{
    configureSELinuxMode();
}

}
