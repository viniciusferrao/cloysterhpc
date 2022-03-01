//
// Created by Vinícius Ferrão on 07/02/22.
//

#include "presenterNetwork.h"

// FIXME: I don't like this code, it's ugly.
//  * Lifecycle may be a problem: what happens if any function throws after
//  adding the Network or Connection? A ghost object will exist without proper
//  data.
//  * If an exception is thrown inside the constructor, the destruction will not
//  be called, leaving garbage on the model.
//  * Try and catch blocks should be added to avoid all those conditions.
// TODO: Just an idea, instead of undoing changes on the model, we may
//  instantiate a Network and a Connection object and after setting up it's
//  attributes we just copy them to the right place or even better, we move it.
//  After the end of this class the temporary objects will be destroyed anyway.

PresenterNetwork::PresenterNetwork(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view,
        Network::Profile profile,
        Network::Type type)
        : m_model(model)
        , m_view(view) {

    // Create the external network
    m_model->addNetwork(profile, type);
    auto& network = m_model->getNetwork(profile);
    LOG_TRACE("Added {} network with type {}",
              magic_enum::enum_name(profile),
              magic_enum::enum_name(type));

    // Create a connection on the headnode
    m_model->getHeadnode().addConnection(m_model->getNetwork(profile));
    auto& connection = m_model->getHeadnode().getConnection(profile);
    LOG_TRACE("Added connection to {} network",
              magic_enum::enum_name(profile));

    // Get the network interface
    const auto& aux = Connection::fetchInterfaces();
    const auto& interface = networkInterfaceSelection(aux);
    connection.setInterface(interface);

    // TODO: Nameservers should be std::vector due to it's random nature
    auto networkDetails = std::to_array<
            std::pair<std::string, std::string>>({
                {Messages::IP::address, Connection::fetchAddress(interface)},
                {Messages::IP::subnetMask, Network::fetchSubnetMask(interface)},
                {Messages::IP::network, Network::fetchAddress(interface)},
                {Messages::IP::gateway, Network::fetchGateway(interface)},
                // Nameserver definitions
                {Messages::Domain::name, Network::fetchDomainName()},
                // TODO: Make it comma or space separated to easy the development
                {Messages::Domain::servers, fmt::format("{}", Network::fetchNameserver()[0])}
            });

    // TODO: Can we use move semantics?
    networkDetails = networkAddress(networkDetails);

#ifndef _NDEBUG_
    networkConfirmation(networkDetails);
#endif

    // Set the gathered data
    size_t i = 0;
    connection.setAddress(networkDetails[i++].second);
    network.setSubnetMask(networkDetails[i++].second);
    network.setAddress(networkDetails[i++].second);
    network.setGateway(networkDetails[i++].second);

    // Domain Data
    network.setDomainName(networkDetails[i++].second);
    network.setNameserver({networkDetails[i++].second}); // This is a std::array

    LOG_TRACE("Added {} connection on headnode: {} -> {}",
              magic_enum::enum_name(profile),
              m_model->getHeadnode().getConnection(profile).getInterface(),
              m_model->getHeadnode().getConnection(profile).getAddress()
    );
}
