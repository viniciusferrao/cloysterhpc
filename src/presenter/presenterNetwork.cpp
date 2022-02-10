//
// Created by Vinícius Ferrão on 07/02/22.
//

#include "presenterNetwork.h"

// FIXME: I don't like this code, it's ugly.
PresenterNetwork::PresenterNetwork(std::unique_ptr<Newt>& view,
                                   std::unique_ptr<Cluster>& model,
                                   Network::Profile profile)
                                   : m_model(model), m_view(view) {

    // Create the external network
    m_model->addNetwork(profile);
    auto& network = m_model->getNetwork(profile);
    LOG_TRACE("Added {} network", Network::getProfileString.at(profile));

    // Create a connection on the headnode
    m_model->getHeadnode().addConnection(m_model->getNetwork(profile));
    auto& connection = m_model->getHeadnode().getConnection(profile);
    LOG_TRACE("Added connection to {} network",
              Network::getProfileString.at(profile));

    // Get the network interface
    const auto& interface = networkInterfaceSelection(Connection::fetchInterfaces());
    connection.setInterface(interface);

    // Build the networkDetails std::vector to feed the view
    m_networkDetails.reserve(6);
    addNetworkDetail("IP Address", Connection::fetchAddress(interface));
    addNetworkDetail("Subnet Mask", Network::fetchSubnetMask(interface));
    addNetworkDetail("Network Address", Network::fetchAddress(interface));
    addNetworkDetail("Gateway", Network::fetchGateway(interface));

    // Nameserver definitions
    addNetworkDetail("Domain name", Network::fetchDomainName());

    // TODO: Make it comma or space separated to easy the development
    const auto nameservers = Network::fetchNameserver();
    for (size_t i = 0 ; const auto& ns : nameservers) {
        m_networkDetails.emplace_back(
                std::make_pair(fmt::format("Nameserver[{}]", i++), ns));
    }

    // Rewrite the data if needed
    m_networkDetails = networkAddress(m_networkDetails);

#ifndef _NDEBUG_
    networkConfirmation(m_networkDetails);
#endif

    // Set the gathered data
    size_t i = 0;
    connection.setAddress(m_networkDetails[i++].second);
    network.setSubnetMask(m_networkDetails[i++].second);
    network.setAddress(m_networkDetails[i++].second);
    network.setGateway(m_networkDetails[i++].second);

    // Domain Data
    network.setDomainName(m_networkDetails[i++].second);
    network.setNameserver({m_networkDetails[i++].second}); // This is a std::vector

    LOG_TRACE("Added {} connection on headnode: {} -> {}",
              Network::getProfileString.at(profile),
              m_model->getHeadnode().getConnection(profile).getInterface(),
              m_model->getHeadnode().getConnection(profile).getAddress()
    );
}

// FIXME: Exception handling is basically useless here.
//        There's an issue where exceptions aren't being handled correctly on
//        the template, and that was circumvented replacing throw for return {};
template<class T>
void PresenterNetwork::addNetworkDetail(const std::string& key, const T& value) {
    try {
        m_networkDetails.emplace_back(std::make_pair(key, value));
    } catch (const std::exception& ex) {
        LOG_WARN(ex.what());
        m_networkDetails.emplace_back(std::make_pair(key, std::string{}));
    }
}

std::string
PresenterNetwork::networkInterfaceSelection(const std::vector<std::string>& interface)
{
    return m_view->listMenu(MSG_TITLE_NETWORK_SETTINGS,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF, interface,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);
}

std::vector<std::string>
PresenterNetwork::networkAddress(const std::vector<std::string>& fields) {
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                             fields,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
}

std::vector<std::pair<std::string,std::string>>
PresenterNetwork::networkAddress(const std::vector<std::pair<std::string,std::string>>& fields) {
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                             fields,
                             MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
}

// TODO: Better implementation
#ifndef _NDEBUG_
void
PresenterNetwork::networkConfirmation(const std::vector<std::pair<std::string, std::string>>& pairs)
{
    return m_view->okCancelMessage("The following network attributes were detected:", pairs);
}
#endif