/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "verification.h"

#include <stdexcept>
#include <unistd.h>

void cloyster::checkEffectiveUserId()
{
    if (geteuid() != 0) {
        throw std::runtime_error(
            "This program must be run with root privileges");
    }
}
