#include "controller.h"
#include "terminalui.h"

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
    headnode->timezone = requestTimezone();
    cluster->timezone = headnode->timezone;

    headnode->locale = requestLocale();
    cluster->locale = headnode->locale;
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
