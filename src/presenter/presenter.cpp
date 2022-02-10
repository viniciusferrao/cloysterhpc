//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "presenterNetwork.h"
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
    try {
        PresenterNetwork external(view, model, Network::Profile::External);
    } catch (const std::exception& e) {
        LOG_WARN("Failed to add {} network: {}",
                 Network::getProfileString.at(Network::Profile::External),
                 e.what());
    }

    try {
        PresenterNetwork management(view, model, Network::Profile::Management);
    } catch (const std::exception& e) {
        LOG_WARN("Failed to add {} network: {}",
                 Network::getProfileString.at(Network::Profile::Management),
                 e.what());
    }



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
