#include <cloysterhpc/services/ansible/roles/nfs.h>
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

namespace cloyster::services::ansible::roles::nfs {

ScriptBuilder installScript(const Role& role, const OS& osinfo)
{
    NFS networkFileSystem = NFS("pub", "/opt/ohpc",
        cluster()
            ->getHeadnode()
            .getConnection(Network::Profile::Management)
            .getAddress(),
        "ro,no_subtree_check");

    return networkFileSystem.installScript(osinfo);
}

}
