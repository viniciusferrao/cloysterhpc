#include <cloysterhpc/services/ansible/roles/dump.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/utils/singleton.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {
using namespace cloyster::utils::singleton;
void dumpPreInstallState()
{
    using namespace cloyster::services::runner;
    const auto opts = cloyster::utils::singleton::options();

    LOG_INFO("Dumping cluster state before the installation begins")

    LOG_INFO("OS");
    shell::cmd("cat /etc/os-release");

    LOG_INFO("Repositories URLs");
    shell::cmd(
        "grep -EH '^(mirrorlist|baseurl)' /etc/yum.repos.d/*.repo || true");

    LOG_INFO("Packages installed");
    shell::cmd("rpm -qa");

    LOG_INFO("Network configuration");
    shell::cmd("ip a");
    shell::cmd("ip link");

    LOG_INFO("Kernel version");
    shell::cmd("uname -a");

    LOG_INFO("Memory");
    shell::cmd("free -m");

    LOG_INFO("Services running");
    shell::cmd("systemctl --no-pager list-units --plain --type=service --all");

    LOG_INFO("Firewall configuration");
    // firewalld may not be running
    shell::cmd("firewall-cmd --list-all-zones || true");

    LOG_INFO("End of cluster state dump");
    opts->maybeStopAfterStep("dump-cluster-state");
}
}

namespace cloyster::services::ansible::roles::dump {

void run(const Role& role)
{
    dumpPreInstallState();
}

}
