/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/verification.h>

#include <stdexcept>
#include <unistd.h>

void cloyster::checkEffectiveUserId()
{
    if (geteuid() != 0) {
        throw std::runtime_error(
            "This program must be run with root privileges");
    }
}

#ifdef BUILD_TESTING
#include <cloysterhpc/tests.h>

TEST_SUITE("Test user privileges")
{
    TEST_CASE("Verify if Cloyster is running with root privileges")
    {
        const auto uid = geteuid();

        if (geteuid() != 0) {
            CHECK_THROWS(cloyster::checkEffectiveUserId());
        } else {
            CHECK_NOTHROW(cloyster::checkEffectiveUserId());
        }
    }
}
#endif