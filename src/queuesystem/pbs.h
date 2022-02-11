//
// Created by Vinícius Ferrão on 11/02/22.
//

#ifndef CLOYSTERHPC_PBS_H
#define CLOYSTERHPC_PBS_H

#include "queuesystem.h"

class PBS : QueueSystem {
public:
    enum class ExecutionPlace {Shared, Scatter};

private:
    ExecutionPlace m_executionPlace = ExecutionPlace::Shared;

public:
    void setExecutionPlace(ExecutionPlace);
    ExecutionPlace getExecutionPlace(void);

};


#endif //CLOYSTERHPC_PBS_H
