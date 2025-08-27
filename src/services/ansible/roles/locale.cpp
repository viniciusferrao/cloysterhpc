#include <cloysterhpc/services/ansible/roles/locale.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {
using namespace cloyster::utils::singleton;
void configureLocale()
{
    LOG_INFO("Setting up locale")

    ::runner()->executeCommand(
        fmt::format("localectl set-locale {}", cluster()->getLocale()));
}
}

namespace cloyster::services::ansible::roles::locale {

void run(const Role& role)
{
    configureLocale();
}

}
