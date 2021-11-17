//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newtViewBase.h"

#include <cstring> /* strlen() */
#include <cstdio> /* sprintf() */
#include <newt.h>

NewtViewBase::NewtViewBase() {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, "PLACEHOLDER: Title text");

    /* Add the default help line in the bottom */
    newtPushHelpLine("PLACEHOLDER: Help message");
    newtRefresh();
}

NewtViewBase::~NewtViewBase() {
    newtFinished();
}

void NewtViewBase::abortInstall () {
    this->~NewtViewBase();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

/* TODO: Refactor this with a C++ dialect */
bool NewtViewBase::hasEmptyField (const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries, and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(nullptr, const_cast<char *>(MSG_BUTTON_OK), message);
            return true;
        }
    }

    return false;
}

void NewtViewBase::helpMessage (const char* message) {
    newtBell();
    newtWinMessage(const_cast<char *>(MSG_TITLE_HELP),
                   const_cast<char *>(MSG_BUTTON_OK),
                   const_cast<char *>(message));
}

/* TODO: Refactor this with a C++ dialect */
#ifdef _DEBUG_
void NewtViewBase::debugEntries(const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 255 chars */
    char message[255] = {};

    /* The ideia on this loop is to read all the available data from *entries,
     * so we add the size of the string in order to give padding for additional
     * data. In the first iteraion strlen() should be zero, and only in the
     * following iterations data will be added to the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++)
        sprintf(message + strlen(message), "%s -> %s\n", entries[i].text, 
                *entries[i].value);
    
    newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), message);
}
#endif
