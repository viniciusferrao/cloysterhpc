#include "presenter.h"
#include "terminalui.h"
#include "connection.h"
#include "network.h"

#include <string>
#include <iostream>

#include "presenterLocaleSelection.h"

Presenter::Presenter (std::unique_ptr<viewTerminalUI> view,
                      std::unique_ptr<Cluster> model) :
                      m_model(std::move(model)),
                      m_view(std::move(view)) {

    m_view->subscribe(this);
}

void Presenter::notifyEvent () {
    std::string name = m_view->getUserText();
    std::string text = m_model->generateText(name);
    m_view->setLabelText(text);
}

#if 0
    /* TODO: Better names for TerminalUI; newt instead? */
    View* view = new TerminalUI();

    auto* locale = new PresenterLocaleSelection(*view);
    locale->write(cluster);
    delete locale;

    delete view;

//    startView();
//    terminalui.~TerminalUI();
}
#endif

void Presenter::startView() {
    /* Timezone */
//    headnode->timezone = requestTimezone();
//    cluster->timezone = headnode->timezone;
//
//    /* Locale */
//    headnode->locale = requestLocale();
//    cluster->locale = headnode->locale;
//
//    /* Hostname and domainname*/
//    std::vector<std::string> fields = requestHostname();
//    headnode->hostname = fields[0];
//    headnode->domainname = fields[1];
//    headnode->fqdn = headnode->hostname + "." + headnode->domainname;

    /* External Network Interface */
    //Connection connection;
//    Network network;
//    network.setProfile(Network::Profile::External);
//    network.setType(Network::Type::Ethernet);
//    network.setInterfacename(requestNetworkInterface());
    //connection.setInterfaceName();
    //headnode->externalConnection.push_back(connection);
//    headnode->externalNetwork.push_back(network);

    /* Get IP addresses */
//    std::vector<std::string> fields = requestNetworkAddress();
//    m_headnode->external[0].setIPAddress(fields[0], fields[1]);

}

std::string Presenter::requestTimezone () {
    /* TODO: Fetch timezones from OS and remove placeholder text */
     const std::vector<std::string> timezones = {
         "America/Sao_Paulo",
         "UTC",
         "Gadific Mean Bolsotime",
         "Chronus",
         "Two blocks ahead"
     };

    return m_terminalui->timezoneSelection(timezones);
}

std::string Presenter::requestLocale () {
    const std::vector<std::string> locales = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C"
    };

    return m_terminalui->localeSelection(locales);
}

/* This method should be renamed or ask just for hostname */
std::vector<std::string> Presenter::requestHostname () {
    const std::vector<std::string> entries = {
        "Hostname",
        "Domain Name"
    };

    return m_terminalui->networkHostnameSelection(entries);
}

/* TODO: Data model is strange, needs fixing. requestNetworkInterface() should
 * be more generic than it's now, not being tied for internal or external
 * interfaces
 */
std::string Presenter::requestNetworkInterface () {
    const std::vector<std::string> netInterfaces = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0",
    };

    return m_terminalui->networkInterfaceSelection(netInterfaces);
}

std::vector<std::string> Presenter::requestNetworkAddress () {
    const std::vector<std::string> networkAddresses = {
        "Headnode IP",
        "Management Network"
    };

    return m_terminalui->networkAddress(networkAddresses);
}
