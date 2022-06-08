/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "newt.h"

bool Newt::yesNoQuestion(const char* title, const char* message,
                         const char* helpMessage) {
    int returnValue;

    question:
    returnValue = newtWinTernary(const_cast<char *>(title),
                                 const_cast<char *>(TUIText::Buttons::yes),
                                 const_cast<char *>(TUIText::Buttons::no),
                                 const_cast<char *>(TUIText::Buttons::help),
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
