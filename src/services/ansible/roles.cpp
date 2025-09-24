#include <algorithm>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/utils/enums.h>
#include <utility>

namespace cloyster::services::ansible::roles {

// NOTE: I little bit of explanation on why this API is like this:
//
// A bit of code context:
//
// - ScriptBuilder is a class that implements an "ansible like" interface,
//   it has methods like .addLineInFile(), .enableService(), .addPackages(),
//   each method generates the required idempotent bash code and accumulates in
//   a vector, then we can run all these commands later. So it decouples *what*
//   runs from *when* it runs and handle the idempotency nicely.
//
// Now a bit of history context:
//
//   The idea was to use ScriptBuilder everywhere such that we decouple
//   the script generation from the environment where it runs. This would
//   help with testing, auditing and would enable static analysis using LLMs
//   and all other cool tricks, but, ..., ScriptBuilder add a translation
//   overhead when converting from shell commands -> scriptbuilder method calls,
//   and we ran out of time to spend on this source code.
//
//   The `foo::run` functions are simple functions that delegates to
//   shell calls, a "quick & dirty" solution that make shell commands ->
//   shell calls mostly 1 to 1, so, fast to use, test and prototype. Ideally,
//   everything would use ScriptBuilder, but we live in a real (not ideal)
//   world. So here are some todos for sometime in the future to improve this
//   code:
//
//   @TODO:
//   - Migrate everything to ScriptBuilder
//   - Decouple script generation from script running
//   - Add a Script class make ScriptBuilder.build return it
//      (s.t. ScriptBuilder becomes a real builder)

class ScriptBuilderRunner {
    ScriptBuilder m_scriptbuilder;

public:
    explicit ScriptBuilderRunner(ScriptBuilder&& builder)
        : m_scriptbuilder(std::move(builder))
    {
    }

    void operator()(const Role& /* role */)
    {
        utils::singleton::runner()->run(m_scriptbuilder);
    }
};

RoleRunnable getRunnable(const Role& role, const models::OS& osinfo)
{
    // wraps ScriptBuilder in a functor
    constexpr auto wrap = [](ScriptBuilder&& scriptbuilder) -> RoleRunnable {
        return ScriptBuilderRunner(std::move(scriptbuilder));
    };

    switch (role.role()) {
        case Roles::CHECK:
            return check::run;
        case Roles::REPOS:
            return repos::run;
        case Roles::NETWORK:
            return network::run;
        case Roles::SSHD:
            return sshd::run;
        case Roles::OFED:
            return ofed::run;
        case Roles::DUMP:
            return dump::run;
        case Roles::LOCALE:
            return locale::run;
        case Roles::FIREWALL:
            return firewall::run;
        case Roles::SELINUX:
            return selinux::run;
        case Roles::NFS:
            return wrap(nfs::installScript(role, osinfo));
        case Roles::QUEUESYSTEM:
            return queuesystem::run;
        case Roles::SLURM:
            return slurm::run;
        case Roles::OHPC:
            return ohpc::run;
        case Roles::PROVISIONER:
            return provisioner::run;
        case Roles::XCAT:
            return xcat::run;
        case Roles::CONFLUENT:
            return confluent::run;
        case Roles::BASE:
            return wrap(base::installScript(role, osinfo));
        case Roles::AUDIT:
            return wrap(audit::installScript(role, osinfo));
        case Roles::AIDE:
            return wrap(aide::installScript(role, osinfo));
        case Roles::FAIL2BAN:
            return wrap(fail2ban::installScript(role, osinfo));
        case Roles::TIMESYNC:
            return wrap(timesync::installScript(role, osinfo));
        case Roles::SPACK:
            return wrap(spack::installScript(role, osinfo));
        default:
            cloyster::functions::abort("Unknown role {}", role.role());
    };

    std::unreachable();
}

void run(const Role& role, const models::OS& osinfo)
{
    LOG_INFO("Loading role {}", role.roleName());
    if (!role.when() || role.when().value()(osinfo)) {
        const auto runnable = getRunnable(role, osinfo);
        LOG_INFO("Executing role {} runnable", role.roleName());
        runnable(role);
    } else {
        LOG_INFO("Skippig role {}, when condition is false", role.roleName());
    }
}

void run(Roles role, const models::OS& osinfo, Role::Vars&& vars,
    Role::Tags&& tags,
    std::optional<std::function<bool(const models::OS& osinfo)>>&& when)
{
    run(
        Role {
            role,
            std::move(tags),
            std::move(vars),
            std::move(when),
        },
        osinfo);
}

void Executor::install()
{
    const auto& roles = utils::singleton::options()->roles;
    const auto osinfo = utils::singleton::os();
    for (const auto& role : roles) {
        auto roleEnum = utils::enums::ofStringExc<Roles>(
            role, utils::enums::Case::Insensitive);
        run(roleEnum, osinfo);
    }
};

}
