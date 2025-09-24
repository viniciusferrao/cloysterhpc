#include <cloysterhpc/services/ansible/roles/firewall.h>
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
void configureFirewall()
{
    LOG_INFO("Setting up firewall")

    if (cluster()->isFirewall()) {
        osservice()->enableService("firewalld");

        // Add the management interface as trusted
        ::runner()->executeCommand(fmt::format(
            "firewall-cmd --permanent --zone=trusted --change-interface={}",
            cluster()
                ->getHeadnode()
                .getConnection(Network::Profile::Management)
                .getInterface()
                .value()));

        // If we have IB, also add its interface as trusted
        if (cluster()->getOFED())
            ::runner()->executeCommand(fmt::format(
                "firewall-cmd --permanent --zone=trusted --change-interface={}",
                cluster()
                    ->getHeadnode()
                    .getConnection(Network::Profile::Application)
                    .getInterface()
                    .value()));

        ::runner()->executeCommand("firewall-cmd --reload");
    } else {
        osservice()->disableService("firewalld");

        LOG_WARN("Firewalld has been disabled")
    }
}

}

namespace cloyster::services::ansible::roles::firewall {

void run(const Role& role) { configureFirewall(); }

}
