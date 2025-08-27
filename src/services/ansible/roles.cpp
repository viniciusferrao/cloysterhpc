#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>
#include <utility>


namespace cloyster::services::ansible::roles {

RoleRunnable getRunnable(const Role& role, const models::OS& osinfo)
{
    // wraps ScriptBuilder in a functor
    constexpr auto wrap = [](const ScriptBuilder& scriptbuilder) -> RoleRunnable {
        return [&](const Role& /* role */) {
            utils::singleton::runner()->run(scriptbuilder);
        };
    };
    const auto roleEnum = utils::enums::ofStringOpt<Roles>(role.m_roleName);
    if (!roleEnum.has_value()) {
        throw std::invalid_argument("Unknown role: " + role.m_roleName);
    }

    switch (roleEnum.value()) {
        case Roles::REPOS:
            return repos::run;
        case Roles::NETWORK:
            return network::run;
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
            std::unreachable();
    };

    std::unreachable();
}

void run(const Role& role, const models::OS& osinfo)
{
    if (!role.m_when || role.m_when.value()(osinfo)) {
        const auto runnable = getRunnable(role, osinfo);
        LOG_INFO("Executing role {}", role.m_roleName);
        runnable(role);
    } else {
        LOG_INFO("Skippig role {}, when condition is false", role.m_roleName);
    }
}

void run(std::string_view roleName, const models::OS& osinfo,
    std::unordered_map<std::string, std::string>&& vars,
    std::optional<std::function<bool(const models::OS& osinfo)>>&& when)
{
    run(
        Role {
            .m_roleName = std::string(roleName),
            .m_vars = std::move(vars),
            .m_when = std::move(when),
        },
        osinfo);
}

void Executor::install() {
    const auto& roles = utils::singleton::options()->roles;
    LOG_INFO("Running roles: {}", fmt::join(roles, ","));
    const auto osinfo = utils::singleton::os();
    for (const auto& role : roles) {
        run(role, osinfo);
    }
};


}
