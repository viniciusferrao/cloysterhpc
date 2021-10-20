#include "controller.h"
#include "terminalui.h"
#include "connection.h"
#include "network.h"

#include <string>
#include <iostream>

Controller::Controller (Cluster& cluster, Headnode& headnode, 
                        TerminalUI& terminalui) {
    this->cluster = &cluster;
    this->headnode = &headnode;
    this->terminalui = &terminalui;

    startView();
    terminalui.~TerminalUI();
}

void Controller::startView() {
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
    Network network;
    network.setProfile(Network::Profile::External);
    network.setType(Network::Type::Ethernet);
    network.setInterfacename(requestNetworkInterface());
    //connection.setInterfaceName();
    //headnode->externalConnection.push_back(connection);
    headnode->externalNetwork.push_back(network);

}

std::string Controller::requestTimezone () {
    const char* const timezones[] = {
        "America/Sao_Paulo",
        "UTC",
        "Gadific Mean Bolsotime",
        "Chronus",
        "Two blocks ahead",
        nullptr
    };
    /* TODO: Fetch timezones from OS and remove placeholder text */
    // std::vector<std::string> timezones = {
    //     "America/Sao_Paulo",
    //     "UTC",
    //     "Gadific Mean Bolsotime",
    //     "Chronus",
    //     "Two blocks ahead"
    // };

    return terminalui->drawTimezoneSelection(timezones);
}

std::string Controller::requestLocale () {
    const char* const locales[] = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C",
        nullptr
    };

    return terminalui->drawLocaleSelection(locales);
}

/* This method should be renamed or ask just for hostname */
std::vector<std::string> Controller::requestHostname () {
    const std::vector<std::string> entries = {
        "Hostname",
        "Domain Name"
    };

    return terminalui->drawNetworkHostnameSelection(entries);
}

/* TODO: Data model is strange, needs fixing. requestNetworkInterface() should
 * be more generic than it's now, not being tied for internal or external
 * interfaces
 */
std::string Controller::requestNetworkInterface () {
    const char* const netInterfaces[] = {
            "eth0",
            "eth1",
            "enp4s0f0",
            "lo",
            "ib0",
            nullptr
    };

    return terminalui->drawNetworkInterfaceSelection(netInterfaces);
}
