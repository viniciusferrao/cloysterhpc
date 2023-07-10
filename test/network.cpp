/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../src/network.h"
#include "../src/connection.cpp"
#include "../src/connection.h"
#include "../src/network.cpp"
#include <boost/asio.hpp>
#include <doctest/doctest.h>

TEST_SUITE("Network setters and getters")
{

    auto testNetwork = Network(Network::Profile::External);

    TEST_CASE("Get m_profile")
    {
        CHECK(testNetwork.getProfile() == Network::Profile::External);
    }

    TEST_CASE("Set and get m_address")
    {
        boost::asio::ip::address address
            = boost::asio::ip::make_address("192.168.0.1");
        testNetwork.setAddress(address);
        CHECK(testNetwork.getAddress() == address);
    }

    TEST_CASE("Set and get m_subnetMask")
    {
        boost::asio::ip::address subnetMask
            = boost::asio::ip::make_address("255.255.255.252");
        testNetwork.setSubnetMask(subnetMask);
        CHECK(testNetwork.getSubnetMask() == subnetMask);
    }

    TEST_CASE("Set and get m_gateway")
    {
        boost::asio::ip::address gateway
            = boost::asio::ip::make_address("0.0.0.0");
        testNetwork.setGateway(gateway);
        CHECK(testNetwork.getGateway() == gateway);
    }

    TEST_CASE("Set and get m_vlan")
    {
        testNetwork.setVLAN(2000);
        CHECK(testNetwork.getVLAN() == 2000);
    }

    TEST_CASE("Set and get m_domainName")
    {
        std::string domainName = "cloyster.com";
        testNetwork.setDomainName(domainName);
        CHECK(testNetwork.getDomainName() == domainName);
    }

    TEST_CASE("Set and get m_nameservers")
    {
        //@TODO must fix the setNameservers function
    }
}

TEST_SUITE("Network fetchers")
{

    Network testNetwork;
    auto testInterface = "enp0s25";

    TEST_CASE("Fetch m_address")
    {
        //@TODO must fix the fetchAddress function
    }

    TEST_CASE("Fetch m_subnetMask")
    {
        CHECK_NOTHROW(testNetwork.fetchSubnetMask(testInterface));
    }

    TEST_CASE("Fetch m_gateway")
    {
        CHECK_NOTHROW(testNetwork.fetchGateway(testInterface));
    }

    TEST_CASE("Fetch m_domainName")
    {
        auto result = testNetwork.fetchDomainName();
        CHECK_FALSE(result.empty());
    }

    TEST_CASE("Fetch m_nameservers")
    {
        auto result = testNetwork.fetchNameservers();
        CHECK_FALSE(result.empty());
    }
}
