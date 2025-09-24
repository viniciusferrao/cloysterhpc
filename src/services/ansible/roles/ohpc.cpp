#include <cloysterhpc/services/ansible/roles/ohpc.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::ohpc {

void run(const Role& role)
{
    LOG_INFO("Installing OpenHPC tools, development libraries, compilers and "
             "MPI stacks");

    auto ohpcPackages = utils::singleton::options()->ohpcPackages;
    utils::singleton::osservice()->install(
        fmt::format("{}", fmt::join(ohpcPackages, " ")));
}

}
