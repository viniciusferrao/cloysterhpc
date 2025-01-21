/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/algorithm/string.hpp>
#include <cloysterhpc/presenter/PresenterNetwork.h>

// Maybe a central NetworkCreator class can be added here?
// The PresenterNetwork receives the NetworkCreator alongside both parameters,
// and just add them to the NetworkInfo. After that, the NetworkInfo, with
// proper methods, adds them to the model

#include <algorithm>

bool NetworkCreator::checkIfProfileExists(Network::Profile profile)
{
    namespace ranges = std::ranges;
    if (auto it = ranges::find_if(
            m_networks, [profile](auto& n) { return n.profile == profile; });
        it != m_networks.end()) {
        return true;
    }

    return false;
}

bool NetworkCreator::addNetworkInformation(NetworkCreatorData&& data)
{
    if (checkIfProfileExists(data.profile)) {
        return false;
    }

    m_networks.push_back(data);
    return true;
}

bool NetworkCreator::checkIfInterfaceRegistered(std::string_view interface)
{
    namespace ranges = std::ranges;
    if (auto it = ranges::find_if(m_networks,
            [interface](auto& n) { return n.interface == interface; });
        it != m_networks.end()) {
        return true;
    }

    return false;
}

std::size_t NetworkCreator::getSelectedInterfaces()
{
    return m_networks.size();
}

void NetworkCreator::saveNetworksToModel(Cluster& model)
{
    for (const auto& net : m_networks) {
        auto netptr = std::make_unique<Network>(net.profile, net.type);
        Connection conn(netptr.get());

        conn.setAddress(net.address);
        conn.setInterface(net.interface);

        netptr->setSubnetMask(net.subnetMask);
        netptr->setAddress(netptr->calculateAddress(conn.getAddress()));
        netptr->setGateway(net.gateway);
        netptr->setDomainName(net.name);
        netptr->setNameservers(net.domains);

        LOG_TRACE("Moved m_network and m_connection into m_model")
        model.addNetwork(std::move(netptr));
        model.getHeadnode().addConnection(std::move(conn));

        // Check moved data
        LOG_DEBUG("Added {} connection on headnode: {} -> {}",
            magic_enum::enum_name(net.profile),
            model.getHeadnode()
                .getConnection(net.profile)
                .getInterface()
                .value(),
            model.getHeadnode()
                .getConnection(net.profile)
                .getAddress()
                .to_string());
    }
}

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
// Most of issues above are being solved:
//  - we moved the network/connection addition to later, after we destroy
//    this class. We move the data using the NetworkCreator class, and create
//    things there.
//
// TODO: Just an idea, instead of undoing changes on the model, we may
//  instantiate a Network and a Connection object and after setting up it's
//  attributes we just copy them to the right place or even better, we move it.
//  After the end of this class the temporary objects will be destroyed anyway.

PresenterNetwork::PresenterNetwork(std::unique_ptr<Cluster>& model,
    std::unique_ptr<Newt>& view, NetworkCreator& nc, Network::Profile profile,
    Network::Type type)
    : Presenter(model, view)
    , m_network(std::make_unique<Network>(profile, type))
{
    LOG_DEBUG("Added {} network with type {}", magic_enum::enum_name(profile),
        magic_enum::enum_name(type));

    LOG_DEBUG("Added connection to {} network", magic_enum::enum_name(profile));

    // TODO: This should be on the header and be constexpr (if possible)
    m_view->message(Messages::title,
        fmt::format(
            "We will now ask questions about your {} ({}) network interface",
            magic_enum::enum_name(profile), magic_enum::enum_name(type))
            .c_str());

    auto interfaces = retrievePossibleInterfaces(nc);

    auto available = interfaces.size() + nc.getSelectedInterfaces();
    if (available < 2) {
        m_view->fatalMessage(Messages::title, Messages::errorInsufficient);
    }

    NetworkCreatorData ncd;
    ncd.type = type;
    ncd.profile = profile;
    createNetwork(interfaces, ncd);
    nc.addNetworkInformation(std::move(ncd));
}

std::vector<std::string> PresenterNetwork::retrievePossibleInterfaces(
    NetworkCreator& nc)
{
    namespace ranges = std::ranges;

    // Get the network interface
    auto ifs = Connection::fetchInterfaces();

    const auto [last, end] = ranges::remove_if(
        ifs, [&nc](auto i) { return nc.checkIfInterfaceRegistered(i); });
    ifs.erase(last, end);
    return ifs;
}

void PresenterNetwork::createNetwork(
    const std::vector<std::string>& interfaceList, NetworkCreatorData& ncd)
{

    std::string interface = networkInterfaceSelection(interfaceList);

    std::vector<address> nameservers = Network::fetchNameservers();
    std::vector<std::string> formattedNameservers;
    for (std::size_t i = 0; i < nameservers.size(); i++) {
        formattedNameservers.emplace_back(nameservers[i].to_string());
    }

    auto networkDetails = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::IP::address,
              Connection::fetchAddress(interface).to_string() },
            { Messages::IP::subnetMask,
                Network::fetchSubnetMask(interface).to_string() },
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
    std::size_t i = 0;

    ncd.interface = interface;
    ncd.address = networkDetails[i++].second;
    ncd.subnetMask = networkDetails[i++].second;
    ncd.gateway = networkDetails[i++].second;

    // Domain Data
    ncd.name = networkDetails[i++].second;
    ncd.domains = nameservers;
}
