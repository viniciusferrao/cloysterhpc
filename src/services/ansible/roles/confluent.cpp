#include <cloysterhpc/services/ansible/roles/confluent.h>
#include <cloysterhpc/services/confluent.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::confluent {

void run(const Role& role)
{
    Confluent confluent;
    confluent.install();
}

}
