//
// Created by Vinícius Ferrão on 27/10/21.
//

#include "viewTerminalUI.h"
#include "messages.h"
#include <iostream>
#include <vector>
#include <string>

viewTerminalUI::viewTerminalUI() {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, "Title text");

    /* Add the default help line in the bottom */
    newtPushHelpLine("Help message");
    newtRefresh();
}

viewTerminalUI::~viewTerminalUI() {
    newtFinished();
}

void viewTerminalUI::subscribe (viewSubscriber* subscriber) {
    m_subscriber = subscriber;
}

void viewTerminalUI::abortInstall () {
    viewTerminalUI::~viewTerminalUI();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

void viewTerminalUI::setLabelText(const std::string &text) {
    newtWinMessage(
            const_cast<char *>("Answer"),
            const_cast<char *>("OK"),
            const_cast<char *>(text.c_str()), NULL);
}

std::vector<std::string> viewTerminalUI::getAnswers() {
    return m_fields;
}

std::string viewTerminalUI::drawListMenu (const char * title, const char * message,
                                      const std::vector<std::string>& items,
                                      const char * helpMessage) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3;

    /* Newt expects a NULL terminated array of C style strings */
    std::vector<const char*> cStrings;
    cStrings.reserve(items.size() + 1);

    for (const auto& item : items) {
        cStrings.push_back(item.c_str());
    }
    cStrings.push_back(nullptr);

#if 1
    /* Goto implementation */
    question:
    returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexDown, flexUp, maxHeightList,
                              const_cast<char **>(cStrings.data()), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL),
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return items[selector];
        case 2:
            //abortInstall();
            exit(-1);
        case 3:
            //drawHelpMessage(helpMessage);
            goto question;
    }
#endif
#if 0
    /* Gotoless implementation */
    for (;;) {
        returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(items), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL),
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return items[selector];
            case 2:
                abortInstall();
            case 3:
                drawHelpMessage(helpMessage);
                continue;
        }
        break; /* for (;;) */
    }
#endif
    return "Error!"; /* We should never reach here */
}

void viewTerminalUI::welcomeMessage () {
    int returnValue;

    /* We start pushing the welcome message */
    newtWinMessage(nullptr, const_cast<char *>(MSG_BUTTON_OK),
                   const_cast<char *>(MSG_WELCOME));

    /* Information about the installation scheme */
    returnValue = newtWinChoice(nullptr, const_cast<char *>(MSG_BUTTON_OK),
                                const_cast<char *>(MSG_BUTTON_CANCEL),
                                const_cast<char *>(MSG_GUIDED_INSTALL));

    switch (returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            break;
        case 2:
            abortInstall();
        default:
            throw;
    }
}

void viewTerminalUI::timezoneSelection
        (const std::vector<std::string>& timezones) {

    m_fields.clear();

    auto selection = drawListMenu(MSG_TITLE_TIME_SETTINGS,
                        MSG_TIME_SETTINGS_TIMEZONE, timezones,
                        MSG_TIME_SETTINGS_TIMEZONE_HELP);

    m_fields.push_back(selection);
    m_subscriber->notifyEvent();
}

void viewTerminalUI::localeSelection
        (const std::vector<std::string>& locales) {

    m_fields.clear();

    auto selection = drawListMenu(MSG_TITLE_LOCALE_SETTINGS,
                        MSG_LOCALE_SETTINGS_LOCALE, locales,
                        MSG_LOCALE_SETTINGS_LOCALE_HELP);

    m_fields.push_back(selection);
    m_subscriber->notifyEvent();
}