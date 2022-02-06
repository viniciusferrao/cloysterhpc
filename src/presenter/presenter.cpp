//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "../services/log.h"

Presenter::Presenter(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model)
                     : m_model(model), m_view(view) {

//    welcomeMessage();
//    LOG_TRACE("Welcome message displayed");
//
//    installInstructions();
//    LOG_TRACE("Install instructions displayed");
//
//    m_model->setTimezone(timezoneSelection(m_model->getTimezone().getAvailableTimezones()));
//    LOG_TRACE("Timezone set to: {}", m_model->getTimezone().getTimezone()); // TODO: Horrible call
//
//    m_model->setLocale(localeSelection({"en_US.UTF-8", "pt_BR.UTF-8", "C"}));
//    LOG_TRACE("Locale set to: {}", m_model->getLocale());
//
//    // TODO: Get rid of aux
//    std::vector<std::string> aux = networkHostnameSelection({"Hostname", "Domain name"});
//    m_model->getHeadnode().setHostname(aux[0]);
//    LOG_TRACE("Returned hostname: {}", aux[0]);
//    LOG_ASSERT(aux[0] == m_model->getHeadnode().getHostname(),
//               "Failed setting hostname");
//
//    m_model->setDomainName(aux[1]);
//    LOG_TRACE("Hostname set to: {}\n", m_model->getHeadnode().getHostname());
//    LOG_TRACE("Domain name set to: {}\n", m_model->getDomainName());
//    LOG_TRACE("FQDN: {}\n", m_model->getHeadnode().getFQDN());

#if 0
    [this](std::vector<std::string> aux) -> void {
        aux = networkHostnameSelection({"Hostname", "Domain name"});
        m_model->getHeadnode().setHostname(aux[0]);
        m_model->setDomainName(aux[1]);
    };
#endif

    // TODO: Under development

    // Create the external network
    m_model->addNetwork();
    LOG_TRACE("Added the external network");

    // Create a connection on the headnode
    m_model->getHeadnode().addConnection(m_model->getNetwork(Network::Profile::External));
    LOG_TRACE("Added the external connection");

    m_model->getHeadnode().getConnection(Network::Profile::External)
                .setInterface(networkInterfaceSelection(Connection::fetchInterfaces()));

    // Fetch the required network/connection data and display on a TUI
    const auto& interface = m_model->getHeadnode()
                                .getConnection(Network::Profile::External)
                                .getInterface();
    std::vector<std::pair<std::string, std::string>> networkDetails;

    const auto nameservers = Network::fetchNameserver();
    networkDetails.reserve(5 + nameservers.size());
    networkDetails.emplace_back(
            std::make_pair("IP Address", Connection::fetchAddress(interface)));
    networkDetails.emplace_back(
            std::make_pair("Subnet Mask", Network::fetchSubnetMask(interface)));
    networkDetails.emplace_back(
            std::make_pair("Network Address", Network::fetchAddress(interface)));
    networkDetails.emplace_back(
            std::make_pair("Gateway", Network::fetchGateway(interface)));

    for (size_t i = 0 ; const auto& ns : nameservers) {
        networkDetails.emplace_back(
            std::make_pair(fmt::format("Nameserver[{}]", i++), ns));
    }

    networkDetails.emplace_back(
            std::make_pair("Domain name", Network::fetchDomainName()));

    networkConfirmation(networkDetails);
    networkDetails.clear();

    // Set the gathered data
    m_model->getHeadnode().getConnection(Network::Profile::External)
                .setAddress(Connection::fetchAddress(interface));
    m_model->getNetwork(Network::Profile::External)
                .setAddress(Network::fetchAddress(interface));
    m_model->getNetwork(Network::Profile::External)
                .setSubnetMask(Network::fetchSubnetMask(interface));
    m_model->getNetwork(Network::Profile::External)
                .setGateway(Network::fetchGateway(interface));


    LOG_TRACE("Added the external connection on headnode: {} -> {}",
            m_model->getHeadnode().getConnection(Network::Profile::External).getInterface(),
            m_model->getHeadnode().getConnection(Network::Profile::External).getAddress()
    );

    // Create the management network
    m_model->addNetwork(Network::Profile::Management);
    LOG_TRACE("Added the management network");

    // Create a connection on the headnode
    m_model->getHeadnode().addConnection(
            m_model->getNetwork(Network::Profile::Management),
                networkInterfaceSelection(Connection::fetchInterfaces()),
                networkAddress({"IP Address"}).front()
    );
    LOG_TRACE("Added the management connection on headnode: {} -> {}",
            m_model->getHeadnode().getConnection(Network::Profile::Management).getInterface(),
            m_model->getHeadnode().getConnection(Network::Profile::Management).getAddress()
    );

    // Destroy the view since we don't need it anymore
    m_view.reset();
}

void Presenter::welcomeMessage() {
    m_view->message(MSG_WELCOME);
}

void Presenter::installInstructions() {
    m_view->okCancelMessage(MSG_GUIDED_INSTALL);
}

std::string Presenter::timezoneSelection(const std::vector<std::string>& timezones) {
    return m_view->listMenu(MSG_TITLE_TIME_SETTINGS,
                            MSG_TIME_SETTINGS_TIMEZONE, timezones,
                            MSG_TIME_SETTINGS_TIMEZONE_HELP);
}

std::string Presenter::localeSelection(const std::vector<std::string>& locales)
{
    return m_view->listMenu(MSG_TITLE_LOCALE_SETTINGS,
                            MSG_LOCALE_SETTINGS_LOCALE, locales,
                            MSG_LOCALE_SETTINGS_LOCALE_HELP);
}

std::vector<std::string>
Presenter::networkHostnameSelection(const std::vector<std::string>& entries)
{
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_HOSTID, entries,
                             MSG_NETWORK_SETTINGS_HOSTID_HELP);
}

std::string
Presenter::networkInterfaceSelection(const std::vector<std::string>& interface)
{
    return m_view->listMenu(MSG_TITLE_NETWORK_SETTINGS,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF, interface,
                            MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);
}

std::vector<std::string>
Presenter::networkAddress(const std::vector<std::string>& addresses) {
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                            MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                            addresses,
                            MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
}

// TODO: Better implementation
void
Presenter::networkConfirmation(const std::vector<std::pair<std::string, std::string>>& pairs)
{
    return m_view->okCancelMessage("The following network attributes were detected:", pairs);
}
