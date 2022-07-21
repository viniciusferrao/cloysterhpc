/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "repos.h"
#include "functions.h"

#include <iostream>

void Repos::createConfigurationFile(const std::string& id, bool enabled,
    const std::string& name, const std::string& baseurl,
    const std::string& metalink, bool gpgcheck, const std::string& gpgkey,
    const std::string& gpgkeyPath)
{
}

void Repos::enable(const std::string& id) { }

void Repos::disable(const std::string& id) { }
