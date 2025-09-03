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

class ScriptBuilderRunner {
    ScriptBuilder m_scriptbuilder;
public:
    explicit ScriptBuilderRunner(ScriptBuilder&& builder) : m_scriptbuilder(std::move(builder)) {}

    void operator()(const Role& /* role */) {
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
            return nfs::run;
        case Roles::QUEUESYSTEM:
            return queuesystem::run;
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
    LOG_INFO("Executing role {}", role.roleName());
    if (!role.when() || role.when().value()(osinfo)) {
        const auto runnable = getRunnable(role, osinfo);
        LOG_INFO("Executing role {} runnable", role.roleName());
        runnable(role);
    } else {
        LOG_INFO("Skippig role {}, when condition is false", role.roleName());
    }
}

void run(Roles role, const models::OS& osinfo,
    Role::Vars&& vars,
    Role::Tags&& tags,
    std::optional<std::function<bool(const models::OS& osinfo)>>&& when)
{
    LOG_INFO("Executing role");
    run(
        Role {
            role,
            std::move(tags),
            std::move(vars),
            std::move(when),
        },
        osinfo);
}

void Executor::install() {
    LOG_INFO("Loading roles from the command line ");
    const auto& roles = utils::singleton::options()->roles;
    const auto osinfo = utils::singleton::os();
    for (const auto& role : roles) {
        LOG_INFO("Loading role: {}", role);
        auto roleEnum = utils::enums::ofStringExc<Roles>(role, utils::enums::Case::Insensitive); 
        run(roleEnum, osinfo);
    }
};


}
