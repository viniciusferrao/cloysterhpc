//
// Created by Vinícius Ferrão on 27/10/21.
//

#include "viewTerminalUI.h"
#include "messages.h"
#include <iostream>

viewTerminalUI::viewTerminalUI() {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, "Title text");

    /* Add the default help line in the bottom */
    newtPushHelpLine("Help message");
    newtRefresh();

    welcomeMessage();
}

viewTerminalUI::~viewTerminalUI() {
    newtFinished();
}

#if 0
void viewTerminalUI::onPushButtonClicked () {
    m_subscriber->notifyButtonPressed();
}
#endif

void viewTerminalUI::subscribe (viewSubscriber* subscriber) {
    m_subscriber = subscriber;
}

void viewTerminalUI::setLabelText(const std::string &text) {
    std::cout << text << std::endl;
}

std::string viewTerminalUI::getUserText() {
    return "Teste getUserText()";
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
            m_subscriber->notifyEvent();
        case 2:
            //abortInstall();
            exit(-1);
        default:
            throw;
    }
}
