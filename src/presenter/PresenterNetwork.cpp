//
// Created by Vinícius Ferrão on 07/02/22.
//

#include "PresenterNetwork.h"

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

PresenterNetwork::PresenterNetwork(std::unique_ptr<Cluster> &model,
                                   std::unique_ptr<Newt> &view,
                                   Network::Profile profile,
                                   Network::Type type)
                                   : Presenter(model, view)
                                   , m_network(profile, type)
                                   , m_connection(m_network)
{
    LOG_TRACE("Added {} network with type {}",
              magic_enum::enum_name(profile),
              magic_enum::enum_name(type));

    LOG_TRACE("Added connection to {} network",
              magic_enum::enum_name(profile));

    // TODO: This should be on the header and be constexpr (if possible)
    m_view->message(Messages::title, fmt::format(
            "We will now ask questions about your {} ({}) network interface",
            magic_enum::enum_name(profile),
            magic_enum::enum_name(type)).c_str()
            );

    createNetwork();
}

void PresenterNetwork::createNetwork()
{
    // Get the network interface
    const auto& aux = Connection::fetchInterfaces();
    const auto& interface = networkInterfaceSelection(aux);
    m_connection.setInterface(interface);

    auto networkDetails = std::to_array<
            std::pair<std::string, std::string>>({
                {Messages::IP::address, Connection::fetchAddress(interface)},
                {Messages::IP::subnetMask, Network::fetchSubnetMask(interface)},
                {Messages::IP::network, Network::fetchAddress(interface)},
                {Messages::IP::gateway, Network::fetchGateway(interface)},
                // Nameserver definitions
                {Messages::Domain::name, Network::fetchDomainName()},
                {Messages::Domain::servers,
                        fmt::format("{}",
                                    fmt::join(Network::fetchNameservers(),
                                              ", "))}
            });

    // TODO: Can we use move semantics?
    networkDetails = networkAddress(networkDetails);

#ifndef _NDEBUG_
    networkConfirmation(networkDetails);
#endif

    // Set the gathered data
    std::size_t i = 0;
    m_connection.setAddress(networkDetails[i++].second);
    m_network.setSubnetMask(networkDetails[i++].second);
    m_network.setAddress(networkDetails[i++].second);
    m_network.setGateway(networkDetails[i++].second);

    // Domain Data
    m_network.setDomainName(networkDetails[i++].second);
    m_network.setNameservers({networkDetails[i++].second}); // This is a std::array

    // Move the data
    m_model->addNetwork(std::move(m_network));
    LOG_TRACE("Hopefully we have moved m_network to m_model");
    m_model->getHeadnode().addConnection(std::move(m_connection));
    LOG_TRACE("Hopefully we have moved m_connection to m_model");

    // FIXME: Shouldn't m_network be unavailable after move? Is it being moved?
    LOG_TRACE("Added {} connection on headnode: {} -> {}",
              magic_enum::enum_name(m_network.getProfile()),
              m_model->getHeadnode()
                      .getConnection(m_network.getProfile())
                      .getInterface()
                      .value(),
              m_model->getHeadnode()
                      .getConnection(m_network.getProfile())
                      .getAddress()
    );
}
