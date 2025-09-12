#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/ansible/roles/queuesystem.h>
#include <cloysterhpc/models/queuesystem.h>
#include <cloysterhpc/models/slurm.h>
#include <cloysterhpc/models/pbs.h>
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
using namespace cloyster::services::ansible;
void configureQueueSystem()
{
    LOG_INFO("Setting up the queue system")

    if (const auto& queue = cluster()->getQueueSystem()) {
        switch (queue.value()->getKind()) {
            case cloyster::models::QueueSystem::Kind::None: {
                __builtin_unreachable();
                break;
            }

            case cloyster::models::QueueSystem::Kind::SLURM: {
                roles::run(roles::Roles::SLURM, os());
                break;
            }

            case cloyster::models::QueueSystem::Kind::PBS: {
                const auto& pbs = dynamic_cast<cloyster::models::PBS*>(queue.value().get());

                osservice()->install("openpbs-server-ohpc");
                osservice()->enableService("pbs");
                ::runner()->executeCommand(
                    "qmgr -c \"set server default_qsub_arguments= -V\"");
                ::runner()->executeCommand(fmt::format(
                    "qmgr -c \"set server resources_default.place={}\"",
                    cloyster::utils::enums::toString<cloyster::models::PBS::ExecutionPlace>(
                        pbs->getExecutionPlace())));
                ::runner()->executeCommand(
                    "qmgr -c \"set server job_history_enable=True\"");
                break;
            }
        }
    }
}

}
namespace cloyster::services::ansible::roles::queuesystem {

void run(const Role& role)
{
    configureQueueSystem();
}

}
