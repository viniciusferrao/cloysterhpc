#include <cloysterhpc/services/ansible/roles/network.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::network {

ScriptBuilder installScript(
    const Role& role, const cloyster::models::OS& osinfo)
{
    throw std::logic_error("Not implemented");
}

}
