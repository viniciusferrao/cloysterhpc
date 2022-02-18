//
// Created by Vinícius Ferrão on 11/02/22.
//

#ifndef CLOYSTERHPC_QUEUESYSTEM_H
#define CLOYSTERHPC_QUEUESYSTEM_H

#include <string>

class QueueSystem {
public:
    enum class Kind { None, SLURM, PBS };

private:
    Kind m_kind = Kind::None;
    std::string_view m_defaultQueue;

public:
    void setKind(Kind kind);
    Kind getKind();

    void setDefaultQueue(std::string_view);
    std::string_view getDefaultQueue();

    virtual ~QueueSystem() = default;
};

#endif //CLOYSTERHPC_QUEUESYSTEM_H
