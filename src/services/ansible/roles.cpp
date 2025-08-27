#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

namespace {
[[noreturn]]
constexpr void TODO() { throw std::logic_error("not implemented"); };
}

namespace cloyster::services::ansible::roles {

RoleRunnable getRunnable(const Role& role, const models::OS& osinfo)
{
    // wraps ScriptBuilder in a functor
    constexpr auto wrap = [](const ScriptBuilder& scriptbuilder) -> RoleRunnable {
        return [&](const Role& /* role */) {
            cloyster::Singleton<IRunner>::get()->run(scriptbuilder);
        };
    };
    if (role.m_roleName == "repos") {
        return repos::run;
    } else if (role.m_roleName == "network") {
        return network::run;
    } else if (role.m_roleName == "locale") {
        return locale::run;
    } else if (role.m_roleName == "firewall") {
        return firewall::run;
    } else if (role.m_roleName == "selinux") {
        return selinux::run;
    } else if (role.m_roleName == "nfs") {
        return nfs::run;
    } else if (role.m_roleName == "queuesystem") {
        return queuesystem::run;
    } else if (role.m_roleName == "ohpc") {
        return ohpc::run;
    } else if (role.m_roleName == "xcat") {
        return xcat::run;
    } else if (role.m_roleName == "confluent") {
        return confluent::run;
    } else if (role.m_roleName == "base") {
        return wrap(base::installScript(role, osinfo));
    } else if (role.m_roleName == "audit") {
        return wrap(audit::installScript(role, osinfo));
    } else if (role.m_roleName == "aide") {
        return wrap(aide::installScript(role, osinfo));
    } else if (role.m_roleName == "fail2ban") {
        return wrap(fail2ban::installScript(role, osinfo));
    } else if (role.m_roleName == "timesync") {
        return wrap(timesync::installScript(role, osinfo));
    } else if (role.m_roleName == "spack") {
        return wrap(spack::installScript(role, osinfo));
    } else {
        throw std::invalid_argument("Unknown role: " + role.m_roleName);
    }
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

}
