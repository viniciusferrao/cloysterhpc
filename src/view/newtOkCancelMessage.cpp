/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/view/newt.h>

void Newt::okCancelMessage(const char* message)
{
    Newt::okCancelMessage(nullptr, message);
}

void Newt::okCancelMessage(const char* title, const char* message)
{
    int returnValue;

    /* Information about the installation scheme */
    returnValue = newtWinChoice(const_cast<char*>(title),
        const_cast<char*>(TUIText::Buttons::ok),
        const_cast<char*>(TUIText::Buttons::cancel),
        const_cast<char*>(message));

    switch (returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            break;
        case 2:
            abort();
            break;
        default:
            throw std::runtime_error(
                "Something happened. Please run the software again");
    }
}

// TODO: Primitive implementation, make it better.
