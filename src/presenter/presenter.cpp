//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "../services/log.h"

Presenter::Presenter(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model)
                     : m_model(model), m_view(view) {

    welcomeMessage();
    LOG_TRACE("Welcome message displayed\n");

    installInstructions();
    LOG_TRACE("Install instructions displayed\n");

    m_model->setTimezone(timezoneSelection(m_model->getTimezone().getAvailableTimezones()));
    LOG_TRACE("Timezone set to: {}\n", m_model->getTimezone().getTimezone()); // TODO: Horrible call

    m_model->setLocale(localeSelection({"en_US.UTF-8", "pt_BR.UTF-8", "C"}));
    LOG_TRACE("Locale set to: {}\n", m_model->getLocale());

    // TODO: Get rid of aux
    std::vector<std::string> aux = networkHostnameSelection({"Hostname", "Domain name"});
    m_model->getHeadnode().setHostname(aux[0]);
    LOG_TRACE("Returned hostname: {}\n", aux[0]);
    LOG_ASSERT(aux[0] == m_model->getHeadnode().getHostname(),
               "Failed setting hostname\n");

    m_model->setDomainName(aux[1]);
    LOG_TRACE("Hostname set to: {}\n", m_model->getHeadnode().getHostname());
    LOG_TRACE("Domain name set to: {}\n", m_model->getDomainName());
    LOG_TRACE("FQDN: {}\n", m_model->getHeadnode().getFQDN());

#if 0
    [this](std::vector<std::string> aux) -> void {
        aux = networkHostnameSelection({"Hostname", "Domain name"});
        m_model->getHeadnode().setHostname(aux[0]);
        m_model->setDomainName(aux[1]);
    };
#endif

    // Network questions
    m_model->getHeadnode().addConnection(
            (m_model->getNetwork(Network::Profile::External)).front(),
            //networkInterfaceSelection({"en0", "eth1", "enp4s0f0"}),
            networkInterfaceSelection(m_model->getHeadnode().getConnection(Network::Profile::External).fetchInterfaces()),
            "192.168.10.10");
            //networkAddress({"Headnode IP", "Management network"}));

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
