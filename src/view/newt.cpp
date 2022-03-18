//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"

#include <cstring> /* strlen() */
#include <cstdio> /* sprintf() */
#include <newt.h>

Newt::Newt()
    : m_flexDown(2)
    , m_flexUp(2) {
    newtInit();
    newtCls();

    // Get the terminal size
    newtGetScreenSize(&m_cols, &m_rows);
    m_suggestedWidth = m_cols / 2;
    m_dataWidth = m_suggestedWidth * 2 / 3;
    // Line count: title, box top border, padding, text message (var),
    // padding (before list), padding (after list), button (4), padding,
    // box bottom border, shadow, status.
    m_maxListHeight = m_rows - 14;

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, TUIText::title);

    /* Add the default help line in the bottom */
    newtPushHelpLine(TUIText::helpLine);
    newtRefresh();
}

Newt::~Newt() {
    newtFinished();
}

void Newt::abort() {
    // TODO: We should only destroy the view and not terminate the application
    this->~Newt();
    LOG_WARN("{}", TUIText::abort);
    std::exit(EXIT_SUCCESS);
}

// TODO: Remove this method; this check must be done outside the view
bool Newt::hasEmptyField(const struct newtWinEntry *entries) {
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
    newtWinMessage(const_cast<char*>(TUIText::Help::title),
                   const_cast<char*>(TUIText::Buttons::ok),
                   const_cast<char*>(message));
}
