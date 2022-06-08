/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SLURM_H_
#define CLOYSTERHPC_SLURM_H_

#include "queuesystem.h"

class SLURM : public QueueSystem {
private:
    bool m_accounting{false};

public:
    explicit SLURM(const Cluster& cluster);

    void installServer();
    void configureServer();
    void enableServer();
    void startServer();
};

#endif // CLOYSTERHPC_SLURM_H_
