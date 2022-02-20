//
// Created by Vinícius Ferrão on 08/01/22.
//

#include "newt.h"

void Newt::okCancelMessage(const char* message) {
    Newt::okCancelMessage(nullptr, message);
}

void Newt::okCancelMessage(const char* title, const char* message) {
    int returnValue;

    /* Information about the installation scheme */
    returnValue = newtWinChoice(const_cast<char*>(title),
                                const_cast<char*>(MSG_BUTTON_OK),
                                const_cast<char*>(MSG_BUTTON_CANCEL),
                                const_cast<char*>(message));

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

// TODO: Primitive implementation, make it better.

