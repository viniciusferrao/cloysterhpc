/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <doctest/doctest.h>

#include <boost/asio.hpp>
#include <cloysterhpc/network.h>
#include <stdexcept>

TEST_SUITE("Network setters and getters")
{
    TEST_CASE("fetchSubnetMask throws for unknown interface")
    {
        CHECK_THROWS_AS(Network::fetchSubnetMask("nonexistent0"),
            std::runtime_error);
    }
}
