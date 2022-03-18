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
    void configureSELinuxMode (Cluster::SELinuxMode);
    void configureFirewall (bool);
    void configureFQDN (const std::string&);
    void configureHostsFile (std::string_view, std::string_view,
                             std::string_view);
    void configureTimezone (std::string_view);
    void configureLocale (const std::string&);

    void disableNetworkManagerDNSOverride (); // This should be on Network
    void configureNetworks(const std::list<Connection>&);

    void runSystemUpdate (bool);
    void installRequiredPackages ();

    void configureRepositories (const std::unique_ptr<Cluster>&);
    void installOpenHPCBase ();
    void configureTimeService (const std::list<Connection>&);
    void configureQueueSystem (const std::unique_ptr<Cluster>&);
    void configureInfiniband (const std::unique_ptr<Cluster>&);
    void configureNetworkFileSystem ();

    void removeMemlockLimits ();
    void installDevelopmentComponents ();

    /* Ancillary functions */
    void disableSELinux ();

public:
    void install(const std::unique_ptr<Cluster>&) override;
};

#endif //CLOYSTER_SHELL_H
