//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"

#include <cstring> /* strlen() */
#include <cstdio> /* sprintf() */
#include <newt.h>

Newt::Newt() {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, TUIText::title);

    /* Add the default help line in the bottom */
    newtPushHelpLine(TUIText::helpLine);
    newtRefresh();
}

Newt::~Newt() {
    newtFinished();
}

void Newt::abortInstall () {
    this->~Newt();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

// TODO: Remove this method; this check must be done outside the view
bool Newt::hasEmptyField (const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries, and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(nullptr, const_cast<char *>(TUIText::Buttons::ok), message);
            return true;
        }
    }

    return false;
}

void Newt::helpMessage (const char* message) {
    newtBell();
    newtWinMessage(const_cast<char *>(MSG_TITLE_HELP),
                   const_cast<char *>(TUIText::Buttons::ok),
                   const_cast<char *>(message));
}
