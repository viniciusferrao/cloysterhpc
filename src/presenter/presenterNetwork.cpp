//
// Created by Vinícius Ferrão on 07/02/22.
//

#include "presenterNetwork.h"

// FIXME: I don't like this code, it's ugly.
PresenterNetwork::PresenterNetwork(std::unique_ptr<Newt>& view,
                                   std::unique_ptr<Cluster>& model,
                                   Network::Profile profile,
                                   Network::Type type)
                                   : m_model(model), m_view(view) {

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
    // TODO: This copy is pathetic, we randomly allocate a huge array.
    //       Also we can't do this, this breaks the Terminal UI interface.
    const auto& aux = Connection::fetchInterfaces();
//    std::array<std::string_view, 30> interfaces;
//    std::copy_n(std::make_move_iterator(aux.begin()), aux.size(), interfaces.begin());

    const auto& interface = networkInterfaceSelection(aux);
    connection.setInterface(interface);

    // Build the networkDetails std::vector to feed the view
//    m_networkDetails.reserve(6);
//    addNetworkDetail("IP Address", Connection::fetchAddress(interface));
//    addNetworkDetail("Subnet Mask", Network::fetchSubnetMask(interface));
//    addNetworkDetail("Network Address", Network::fetchAddress(interface));
//    addNetworkDetail("Gateway", Network::fetchGateway(interface));

    // std::array version
    // TODO: Nameservers should be std::vector due to it's random nature
    auto networkDetails = std::to_array<
            std::pair<std::string, std::string>>({
                {"IP Address", Connection::fetchAddress(interface)},
                {"Subnet Mask", Network::fetchSubnetMask(interface)},
                {"Network Address", Network::fetchAddress(interface)},
                {"Gateway", Network::fetchGateway(interface)},
                // Nameserver definitions
                {"Domain name", Network::fetchDomainName()},
                // TODO: Make it comma or space separated to easy the development
                {"Nameservers", fmt::format("{}", Network::fetchNameserver()[0])}
            });

    // Nameserver definitions
    //addNetworkDetail("Domain name", Network::fetchDomainName());

    // TODO: Make it comma or space separated to easy the development
//    const auto nameservers = Network::fetchNameserver();
//    for (size_t i = 0 ; const auto& ns : nameservers) {
//        m_networkDetails.emplace_back(
//                std::make_pair(fmt::format("Nameserver[{}]", i++), ns));
//    }

    // Rewrite the data if needed
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

template<typename T>
std::string
PresenterNetwork::networkInterfaceSelection(const T& interfaces)
{
    return std::string {m_view->listMenu(MSG_TITLE_NETWORK_SETTINGS,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF, interfaces,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP)};
}

template<size_t N>
std::array<std::pair<std::string,std::string>, N>
PresenterNetwork::networkAddress(const std::array<std::pair<std::string,std::string>, N>& fields) {
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                             fields,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
}

// TODO: Better implementation
#ifndef _NDEBUG_
template <size_t N>
void
PresenterNetwork::networkConfirmation(const std::array<std::pair<std::string, std::string>, N>& pairs)
{
    return m_view->okCancelMessage("The following network attributes were detected:", pairs);
}
#endif
