#include <cloysterhpc/services/ansible/roles/check.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <fmt/core.h>

namespace cloyster::services::ansible::roles::check {

void run(const Role& role)
{
    // TODO
    // Implement checks to run before the installation
    //
    // - lvm must be used with thin provisioning
    // - enough space disk
    // - answerfile validation
    // - internet connection?
    // - no swap?
}

} // namespace cloyster::services::ansible::roles::check
