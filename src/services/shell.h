//
// Created by Vinícius Ferrão on 31/10/21.
//
#ifndef CLOYSTER_SHELL_H
#define CLOYSTER_SHELL_H

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

#endif //CLOYSTER_SHELL_H
