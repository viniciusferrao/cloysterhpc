/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <doctest/doctest.h>

#include <boost/asio.hpp>
#include <cloysterhpc/network.h>
#include <stdexcept>

// @FIXME: Network class should be move to the proper namespace
TEST_SUITE("cloyster::services::network")
{
    TEST_CASE("fetchSubnetMask throws for unknown interface")
    {
        CHECK_THROWS_AS(static_cast<void>(Network::fetchSubnetMask("nonexistent0")),
            std::runtime_error);
    }
}
