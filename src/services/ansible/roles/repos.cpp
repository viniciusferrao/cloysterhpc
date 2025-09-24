#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/ansible/roles/repos.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::repos {

void run(const Role& role)
{
    const auto& osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    auto repos = cloyster::Singleton<services::repos::RepoManager>::get();
    repos->initializeDefaultRepositories();
}

}
