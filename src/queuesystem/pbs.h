/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PBS_H_
#define CLOYSTERHPC_PBS_H_

#include "queuesystem.h"

class PBS : public QueueSystem {
public:
    enum class ExecutionPlace { Shared, Scatter };

private:
    ExecutionPlace m_executionPlace = ExecutionPlace::Shared;

public:
    void setExecutionPlace(ExecutionPlace);
    ExecutionPlace getExecutionPlace(void);

public:
    explicit PBS(const Cluster& cluster);
};

#endif // CLOYSTERHPC_PBS_H_
