//
// Created by Vinícius Ferrão on 11/02/22.
//

#ifndef CLOYSTERHPC_QUEUESYSTEM_H
#define CLOYSTERHPC_QUEUESYSTEM_H

#include <string>

class QueueSystem {
public:
    enum class Kind { None, SLURM, PBS };
    inline static const std::unordered_map<Kind, std::string> getKindString = { // NOLINT
            {Kind::None,   "None"},
            {Kind::SLURM, "SLURM"},
            {Kind::PBS,     "PBS"}
    };

private:
    Kind m_kind;
    std::string_view m_defaultQueue;

public:
    void setKind(Kind kind);
    Kind getKind(void);

    virtual void setDefaultQueue(std::string_view);
    virtual std::string_view getDefaultQueue(void);
};

#endif //CLOYSTERHPC_QUEUESYSTEM_H
