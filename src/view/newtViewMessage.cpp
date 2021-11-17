//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newtViewMessage.h"

void NewtViewMessage::draw () {
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
            break;
        default:
            throw;
    }
}
