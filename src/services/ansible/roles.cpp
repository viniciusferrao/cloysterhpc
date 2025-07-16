#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

namespace cloyster::services::ansible::roles {

ScriptBuilder installScript(const Role& role, const models::OS& osinfo)
{
    if (role.m_roleName == "base") {
        return base::installScript(role, osinfo);
    } else if (role.m_roleName == "audit") {
        return audit::installScript(role, osinfo);
    } else if (role.m_roleName == "aide") {
        return aide::installScript(role, osinfo);
    } else if (role.m_roleName == "fail2ban") {
        return fail2ban::installScript(role, osinfo);
    } else if (role.m_roleName == "timesync") {
        return timesync::installScript(role, osinfo);
    } else if (role.m_roleName == "spack") {
        return spack::installScript(role, osinfo);
    } else {
        throw std::invalid_argument("Unknown role: " + role.m_roleName);
    }
}

void run(const Role& role, const models::OS& osinfo)
{
    if (!role.m_when || role.m_when.value()(osinfo)) {
        const auto runner = cloyster::Singleton<IRunner>::get();
        const auto script = installScript(role, osinfo);
        LOG_INFO("Executing role {}", role.m_roleName);
        runner->run(script);
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
