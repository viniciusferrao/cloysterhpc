#include <cloysterhpc/services/ansible/roles/provisioner.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::provisioner {

void run(const Role& role)
{
    throw std::logic_error("Not implemented");
}

}
