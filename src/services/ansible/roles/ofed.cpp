#include <cloysterhpc/ofed.h>
#include <cloysterhpc/services/ansible/roles/ofed.h>
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

void configureInfiniband()
{
    if (const auto& ofed = cluster()->getOFED()) {
        LOG_INFO("Setting up Infiniband support")
        ofed->install(); // shared pointer
    }
}

}

namespace cloyster::services::ansible::roles::ofed {

void run(const Role& role)
{
    LOG_INFO("Setting up Infiniband, use `--skip infiniband` to skip");
    if (cloyster::utils::singleton::options()->shouldSkip("infiniband")) {
        return;
    }
    configureInfiniband();
}

}
