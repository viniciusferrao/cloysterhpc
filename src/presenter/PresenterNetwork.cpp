/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PresenterNetwork.h"
#include <boost/algorithm/string.hpp>

// FIXME: I don't like this code, it's ugly.
//  *** The following ideas were implemented, although not properly tested, so
//  *** we leave this comment here.
//  * Lifecycle may be a problem: what happens if any function throws after
//  adding the Network or Connection? A ghost object will exist without proper
//  data.
//  * If an exception is thrown inside the constructor, the destructor will not
//  be called, leaving garbage on the model.
//  * Try and catch blocks should be added to avoid all those conditions.
//
// TODO: Just an idea, instead of undoing changes on the model, we may
//  instantiate a Network and a Connection object and after setting up it's
//  attributes we just copy them to the right place or even better, we move it.
//  After the end of this class the temporary objects will be destroyed anyway.

PresenterNetwork::PresenterNetwork(std::unique_ptr<Cluster>& model,
    std::unique_ptr<Newt>& view, Network::Profile profile, Network::Type type)
    : Presenter(model, view)
    , m_network(std::make_unique<Network>(profile, type))
    , m_connection(m_network.get())
{
    LOG_DEBUG("Added {} network with type {}",
        magic_enum::enum_name(m_network->getProfile()),
        magic_enum::enum_name(m_network->getType()));

    LOG_DEBUG("Added connection to {} network",
        magic_enum::enum_name(m_connection.getNetwork()->getProfile()));

    // TODO: This should be on the header and be constexpr (if possible)
    m_view->message(Messages::title,
        fmt::format(
            "We will now ask questions about your {} ({}) network interface",
            magic_enum::enum_name(profile), magic_enum::enum_name(type))
            .c_str());

    createNetwork();
}

void PresenterNetwork::createNetwork()
{
    // Get the network interface
    const auto& aux = Connection::fetchInterfaces();
    const auto& interface = networkInterfaceSelection(aux);
    m_connection.setInterface(interface);

    std::vector<address> nameservers = Network::fetchNameservers();
    std::vector<std::string> formattedNameservers;
    for (int i = 0; i < nameservers.size(); i++) {
        formattedNameservers.emplace_back(nameservers[i].to_string());
    }

    auto networkDetails = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::IP::address,
              Connection::fetchAddress(interface).to_string() },
            { Messages::IP::subnetMask,
                Network::fetchSubnetMask(interface).to_string() },
            { Messages::IP::network,
                Network::fetchAddress(interface).to_string() },
            { Messages::IP::gateway,
                Network::fetchGateway(interface).to_string() },
            // Nameserver definitions
            { Messages::Domain::name, Network::fetchDomainName() },
            { Messages::Domain::servers,
                fmt::format("{}", fmt::join(formattedNameservers, ", ")) } });

    // TODO: Can we use move semantics?
    networkDetails = networkAddress(networkDetails);

#ifndef NDEBUG
    networkConfirmation(networkDetails);
#endif

    // Set the gathered data
    m_connection.setAddress(Connection::fetchAddress(interface));
    m_network->setSubnetMask(Network::fetchSubnetMask(interface));
    m_network->setAddress(Network::fetchAddress(interface));
    m_network->setGateway(Network::fetchGateway(interface));

    // Domain Data
    m_network->setDomainName(Network::fetchDomainName());

    m_network->setNameservers(nameservers); // TODO: std::move

#ifndef NDEBUG
    [[maybe_unused]] const auto& profile = m_network->getProfile();
#endif

    // Move the data
    m_model->addNetwork(std::move(m_network));
    LOG_TRACE("Hopefully we have moved m_network to m_model");
    m_model->getHeadnode().addConnection(std::move(m_connection));
    LOG_TRACE("Hopefully we have moved m_connection to m_model");

    // Check moved data
    LOG_DEBUG("Added {} connection on headnode: {} -> {}",
        magic_enum::enum_name(profile),
        m_model->getHeadnode().getConnection(profile).getInterface().value(),
        m_model->getHeadnode().getConnection(profile).getAddress().to_string());
}
