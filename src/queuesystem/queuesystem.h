//
// Created by Vinícius Ferrão on 11/02/22.
//

#ifndef CLOYSTERHPC_QUEUESYSTEM_H
#define CLOYSTERHPC_QUEUESYSTEM_H

#include <string>
#include <fmt/format.h>

#include "../functions.h"

// Forward declaration of Cluster
class Cluster;

class QueueSystem {
public:
    enum class Kind { None, SLURM, PBS };

private:
    Kind m_kind = Kind::None;
    std::string_view m_defaultQueue;

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

#endif //CLOYSTERHPC_QUEUESYSTEM_H
