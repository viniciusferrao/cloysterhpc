#include <cloysterhpc/services/IService.h>
#include <cloysterhpc/services/log.h>
#include <stdexcept>

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
    LOG_TRACE("service: enabling {}", m_name);

    auto ret = callObjectFunctionArray("EnableUnitFiles", false, true)
                   .getPair<bool, EnableRType>();
    const auto& [_install, retvec] = ret;

    if (retvec.empty()) {
        LOG_WARN("service {} already enabled", m_name);
    }
}

void IService::disable()
{
    LOG_TRACE("service: disabling {}", m_name);

    auto ret = callObjectFunctionArray("DisableUnitFiles", false, true)
                   .get<EnableRType>();

    if (ret.empty()) {
        LOG_WARN("service {} already disabled", m_name);
    }
}

void IService::start()
{
    LOG_TRACE("service: starting {}", m_name);
    (void)callObjectFunction("StartUnit", "replace");
}

void IService::restart()
{
    LOG_TRACE("service: restarting {}", m_name);
    (void)callObjectFunction("RestartUnit", "replace");
}

void IService::stop()
{
    LOG_TRACE("service: stopping {}", m_name);
    (void)callObjectFunction("StopUnit", "replace");
}
