#include "terminalui.hpp"
#include "messages.hpp"

#include <cstddef>
#include <iostream>

/* Constructor */
TerminalUI::TerminalUI (void) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();

    beginInstall();
}

TerminalUI::~TerminalUI (void) {
    newtFinished();
}

void TerminalUI::abortInstall (void) {
    TerminalUI::~TerminalUI();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

void TerminalUI::beginInstall (void) {
    welcomeMessage();
#if 0
    timeSettings(cluster);
    localeSettings(cluster);
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