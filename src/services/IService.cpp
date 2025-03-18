#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/services/IService.h>
#include <cloysterhpc/services/log.h>
#include <stdexcept>

/* BUG: Refactor:
 * Legacy casting.
 * Dry run is a band-aid solution.
 * Variables could be const.
 * Variables name are not the best ones.
 * Check grammar.
 * Warnings during compilation.
 */

using EnableRType
    = std::vector<sdbus::Struct<std::string, std::string, std::string>>;

bool IService::handleException(const sdbus::Error& e, const std::string_view fn)
{
    if (e.getName() == "org.freedesktop.systemd1.NoSuchUnit") {
        throw daemon_exception(fmt::format(
            "Daemon {} not found (NoSuchUnit while calling {})", m_name, fn));
    }

    return false;
}

void IService::enable()
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would have enabled the service {}", m_name)
        return;
    }

    LOG_TRACE("service: enabling {}", m_name);

    auto ret = callObjectFunctionArray("EnableUnitFiles", false, true);
    if (!ret.has_value()) {
        LOG_ERROR(
            "callObjectFunctionArray returned none for service {}", m_name);
        return;
    }
    const auto& [_install, retvec] = (*ret).getPair<bool, EnableRType>();

    if (retvec.empty()) {
        LOG_WARN("service {} already enabled", m_name);
    }
}

void IService::disable()
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would have disabled the service {}", m_name)
        return;
    }

    LOG_TRACE("service: disabling {}", m_name);

    auto ret = callObjectFunctionArray("DisableUnitFiles", false, true);
    if (!ret.has_value()) {
        LOG_ERROR("callObjectFunctionArray returned none, service {}", m_name);
        return;
    };

    if ((*ret).get<EnableRType>().empty()) {
        LOG_WARN("service {} already disabled", m_name);
    }
}

void IService::start()
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would have started the service {}", m_name)
        return;
    }

    LOG_TRACE("service: starting {}", m_name);
    callObjectFunction("StartUnit", "replace");
}

void IService::restart()
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would have restarted the service {}", m_name)
        return;
    }

    LOG_TRACE("service: restarting {}", m_name);
    callObjectFunction("RestartUnit", "replace");
}

void IService::stop()
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would have stopped the service {}", m_name)
        return;
    }

    LOG_TRACE("service: stopping {}", m_name);
    callObjectFunction("StopUnit", "replace");
}
