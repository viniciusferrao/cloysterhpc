#include "terminalui.hpp"
#include "messages.hpp"
#include "headnode.hpp"

#include <cstddef>
#include <iostream>

/* Constructor */
TerminalUI::TerminalUI (Headnode *headnode) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();

    beginInstall(headnode);
}

/* Destructor */
TerminalUI::~TerminalUI (void) {
    newtFinished();
}

void TerminalUI::abortInstall (void) {
    TerminalUI::~TerminalUI();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

/* This is the default help action function, that should be called whatever the
 * operator asks for help.
 * The nature of the function relies on const char* for safety reasons but the
 * underling function requires char *; so we basically need to duplicate the
 * string to a temporary one just to display the message.
 */
void TerminalUI::helpMessage (const char* message) {
    newtBell();
    newtWinMessage(const_cast<char *>(MSG_TITLE_HELP),
                   const_cast<char *>(MSG_BUTTON_OK),
                   const_cast<char *>(message));
}

void TerminalUI::beginInstall (Headnode *headnode) {
//    welcomeMessage();
    timeSettings(headnode);
//    localeSettings(headnode);
#if 0
    networkSettings(cluster);
    directoryServicesSettings(cluster);
    nodeSettings(cluster);
    infinibandSettings(cluster);
    queueSystemSettings(cluster);
    postfixSettings(cluster);
    updateSystem(cluster);
    remoteAccessSettings(cluster);
#endif
}

void TerminalUI::welcomeMessage (void) {
    int returnValue;

    /* We start pushing the welcome message */
    newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), 
                    const_cast<char *>(MSG_WELCOME));

    /* Information about the installation scheme */
    returnValue = newtWinChoice(NULL, const_cast<char *>(MSG_BUTTON_OK), 
                                const_cast<char *>(MSG_BUTTON_CANCEL),
                                const_cast<char *>(MSG_GUIDED_INSTALL));

    switch (returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            break;
        case 2:
            abortInstall();
    }
}

void TerminalUI::timeSettings (Headnode *headnode) {
    int returnValue;
    int selector = 0;

    /* This is a placeholder const until we figure out how to fetch the list of
     * supported timezones from the OS instead.
     */
    const char* const timezones[] = {
        "America/Sao_Paulo",
        "UTC",
        "Gadific Mean Bolsotime",
        "Chronus",
        "Two blocks ahead",
        NULL 
    };

    headnode->timezone = drawListMenu(MSG_TITLE_TIME_SETTINGS,
                                      MSG_TIME_SETTINGS_TIMEZONE, timezones,
                                      MSG_TIME_SETTINGS_TIMEZONE_HELP);

#if 0 
    for (;;) {
    returnValue = newtWinMenu(const_cast<char *>(MSG_TITLE_TIME_SETTINGS),
                              const_cast<char *>(MSG_TIME_SETTINGS_TIMEZONE),
                              50, 5, 5, 3, (char**) timezones, &selector, 
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL), 
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            headnode->timezone = timezones[selector];
            break;
        case 2:
            abortInstall();
        case 3:
            helpMessage(MSG_TIME_SETTINGS_TIMEZONE_HELP);
            continue;
    } break; }
#endif

}

void TerminalUI::localeSettings (Headnode *headnode) {
    int returnValue;
    int selector = 0;

    /* This is a placeholder const until we figure out how to fetch the list of
     * supported locales from the OS instead.
     */
    const char* const locales[] = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C",
        NULL
    };

    headnode->timezone = drawListMenu(MSG_TITLE_LOCALE_SETTINGS,
                                      MSG_LOCALE_SETTINGS_LOCALE, locales,
                                      MSG_LOCALE_SETTINGS_LOCALE_HELP);
}

std::string TerminalUI::drawListMenu (const char * title, const char * message,
                                      const char * const * itens,
                                      const char * helpText) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3; 

    /* Goto implementation */
    question0:
    returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(itens), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL), 
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return itens[selector];
        case 2:
            abortInstall();
        case 3:
            helpMessage(helpText);
            goto question0;
    }

#if 0
    /* Gotoless implementation */
    for (;;) {
        returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(itens), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL), 
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return itens[selector];
            case 2:
                abortInstall();
            case 3:
                helpMessage(helpText);
                continue;
        }
        break; /* for (;;) */
    }
#endif

}
