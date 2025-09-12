/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SHELL_H_
#define CLOYSTERHPC_SHELL_H_

#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/execution.h>

namespace cloyster::services {

using cloyster::models::Cluster;
/**
 * @class Shell
 * @brief Manages the configuration and installation processes on a cluster.
 *
 * This class provides functionalities for configuring various system settings,
 * installing required packages, and setting up cluster-specific services.
 *
 * Note this class delegates the logic to the ansible roles
 */
class Shell final : public Execution {
public:
    /**
     * @brief Installs and configures the system.
     *
     * This function performs the installation and configuration processes.
     */
    void install() override;

    Shell();
};

};

#endif // CLOYSTERHPC_SHELL_H_
