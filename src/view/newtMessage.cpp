/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "newt.h"

void Newt::message(const char* message) { Newt::message(nullptr, message); }

void Newt::message(const char* title, const char* message)
{
    /* We start pushing the welcome message */
    newtWinMessage(const_cast<char*>(title),
        const_cast<char*>(TUIText::Buttons::ok), const_cast<char*>(message));
}
