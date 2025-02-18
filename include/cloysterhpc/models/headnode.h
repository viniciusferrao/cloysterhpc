/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_HEADNODE_H_
#define CLOYSTERHPC_HEADNODE_H_

#include <list>
#include <memory>
#include <string>

#include <cloysterhpc/connection.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/models/server.h>
#include <cloysterhpc/network.h>

namespace cloyster::models {
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

}; // namespace cloyster::models

#endif // CLOYSTERHPC_HEADNODE_H_
