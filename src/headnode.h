/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_HEADNODE_H_
#define CLOYSTERHPC_HEADNODE_H_

#include <list>
#include <memory>
#include <string>

#include "connection.h"
#include "network.h"
#include "os.h"
#include "server.h"

class Headnode : public Server {
public:
    enum class BootTarget { Text, Graphical };

private:
    BootTarget m_bootTarget;

private:
    void discoverNames();

public:
    Headnode();

    [[nodiscard]] BootTarget getBootTarget() const;
    void setBootTarget(BootTarget bootTarget);
};

#endif // CLOYSTERHPC_HEADNODE_H_
