#include <cloysterhpc/services/ansible/roles/xcat.h>
#include <cloysterhpc/services/xcat.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/NFS.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace {
using namespace cloyster::utils::singleton;
}

namespace cloyster::services::ansible::roles::xcat {

void run(const Role& role)
{
    XCAT xcat;
    xcat.install();
}

}
