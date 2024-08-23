/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SLURM_H_
#define CLOYSTERHPC_SLURM_H_

#include <cloysterhpc/queuesystem/queuesystem.h>

/**
 * @class SLURM
 * @brief Manages SLURM server installation and configuration.
 */
class SLURM : public QueueSystem {
private:
    bool m_accounting { false };

public:
    explicit SLURM(const Cluster& cluster);

    /**
     * @brief Installs the SLURM server package on the system.
     */
    void installServer();

    /**
     * @brief Configures the SLURM server.
     */
    void configureServer();

    /**
     * @brief Enables the SLURM server to start at boot.
     */
    void enableServer();

    /**
     * @brief Starts the SLURM server.
     */
    void startServer();
};

#endif // CLOYSTERHPC_SLURM_H_
