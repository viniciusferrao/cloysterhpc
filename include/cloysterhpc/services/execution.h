/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_EXECUTION_H_
#define CLOYSTERHPC_EXECUTION_H_

#include <string>

#include <cloysterhpc/models/cluster.h>

class Execution {
public:
    virtual ~Execution() = default;

    // virtual void runCommand(const std::string&) = 0;
    virtual void install() = 0;
};

#endif // CLOYSTERHPC_EXECUTION_H_
