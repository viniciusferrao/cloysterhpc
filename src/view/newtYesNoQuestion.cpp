//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"

bool Newt::yesNoQuestion(const char * title, const char * message,
                         const char * helpMessage) {
    int returnValue;

    question:
    returnValue = newtWinTernary(const_cast<char *>(title),
                                 const_cast<char *>(MSG_BUTTON_YES),
                                 const_cast<char *>(MSG_BUTTON_NO),
                                 const_cast<char *>(MSG_BUTTON_HELP),
                                 const_cast<char *>(message), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return true;
        case 2:
            return false;
        case 3:
            this->helpMessage(helpMessage);
            goto question;
        default:
            __builtin_unreachable();
    }
}
