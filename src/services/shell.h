/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SHELL_H_
#define CLOYSTERHPC_SHELL_H_

#include "../include/magic_enum.hpp"

#include "execution.h"
#include "../cluster.h"
#include "provisioner.h"

class Shell : public Execution {
private:
    const std::unique_ptr<Cluster> &m_cluster;

private:
    void configureSELinuxMode();
    void configureFirewall();
    void configureFQDN();
    void configureHostsFile();
    void configureTimezone();
    void configureLocale();

    void disableNetworkManagerDNSOverride(); // This should be on Network
    void configureNetworks(const std::list<Connection>&);

    void runSystemUpdate();
    void installRequiredPackages();

    void configureRepositories();
    void installOpenHPCBase();
    void configureTimeService (const std::list<Connection>&);
    void configureQueueSystem();
    void configureInfiniband();
    void configureNetworkFileSystem ();

    void removeMemlockLimits();
    void installDevelopmentComponents();

    /* Ancillary functions */
    void disableSELinux();

public:
    // FIXME: Guideline: Don’t use a const unique_ptr& as a parameter;
    //  use widget* instead.
    explicit Shell(const std::unique_ptr<Cluster> &cluster);
    void install() override;
};

#endif // CLOYSTERHPC_SHELL_H_
