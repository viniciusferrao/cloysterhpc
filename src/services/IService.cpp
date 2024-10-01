#include <cloysterhpc/services/IService.h>
#include <cloysterhpc/services/log.h>
#include <stdexcept>

using EnableRType
    = std::vector<sdbus::Struct<std::string, std::string, std::string>>;

void IService::enable()
{
    LOG_TRACE("service: enabling {}", m_name);

    auto ret = callObjectFunctionArray("EnableUnitFiles", false, true)
                   .getPair<bool, EnableRType>();
    const auto& [_install, retvec] = ret;

    if (retvec.empty()) {
        throw std::runtime_error {
            "Service enable failed, dbus call returned <= 0"
        };
    }
}

void IService::disable()
{
    LOG_TRACE("service: disabling {}", m_name);

    auto ret = callObjectFunctionArray("DisableUnitFiles", false, true)
                   .get<EnableRType>();

    if (ret.empty()) {
        throw std::runtime_error {
            "Service disable failed, dbus call returned <= 0"
        };
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
