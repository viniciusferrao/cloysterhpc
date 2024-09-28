#include <cloysterhpc/services/IService.h>

void IService::enable()
{
    (void)callObjectFunction("EnableUnitFiles", false, true);
}

void IService::disable()
{
    (void)callObjectFunction("DisableUnitFiles", false, true);
}

void IService::start() { (void)callObjectFunction("StartUnit", "replace"); }

void IService::restart() { (void)callObjectFunction("RestartUnit", "replace"); }

void IService::stop() { (void)callObjectFunction("StopUnit", "replace"); }
