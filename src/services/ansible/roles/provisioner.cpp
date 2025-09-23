#include <cloysterhpc/services/ansible/roles/provisioner.h>
#include <cloysterhpc/services/ansible/roles.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/utils/singleton.h>
#include <cloysterhpc/functions.h>

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
    const auto provisioner = utils::singleton::answerfile()->system.provisioner;
    const auto osinfo = utils::singleton::os();
    if (provisioner == "confluent") {
        roles::run(Roles::CONFLUENT, osinfo);
    } else if  (provisioner == "xcat") {
        roles::run(Roles::XCAT, osinfo);
    } else {
        cloyster::functions::abort("Expecing xcat or confluent at "
                                   "[system].provisioner, found: {}, at {}",
            provisioner, utils::singleton::answerfile()->path());
    }
}

}
