/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_QUEUESYSTEM_H_
#define CLOYSTERHPC_QUEUESYSTEM_H_

#include <fmt/format.h>
#include <string>

#include "../functions.h"

// Forward declaration of Cluster
class Cluster;

class QueueSystem {
public:
    enum class Kind { None, SLURM, PBS };

private:
    Kind m_kind = Kind::None;
    std::string m_defaultQueue;

protected:
    const Cluster& m_cluster;

public:
    void setKind(Kind kind);
    Kind getKind();

    void setDefaultQueue(std::string_view);
    std::string_view getDefaultQueue();

    explicit QueueSystem(const Cluster& cluster);
    virtual ~QueueSystem() = default;
};

#endif // CLOYSTERHPC_QUEUESYSTEM_H_
