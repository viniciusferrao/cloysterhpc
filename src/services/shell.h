//
// Created by Vinícius Ferrão on 31/10/21.
//

#ifndef CLOYSTER_SHELL_H
#define CLOYSTER_SHELL_H

#include "execution.h"
#include "../cluster.h"

class Shell : public Execution {
public: /* TODO: Make it private, only here to run xCAT provisioner class */
    void runCommand(const std::string&);

private:
    void configureSELinuxMode (Cluster::SELinuxMode);
    void configureFirewall (bool);
    void configureFQDN (const std::string&);
    void configureHostsFile (std::string_view, std::string_view,
                             std::string_view);
    void configureTimezone (std::string);
    void configureLocale (std::string);
    void runSystemUpdate (bool);
    void installRequiredPackages ();




    void configureRepositories (const std::unique_ptr<Cluster>&);
    void installProvisioningServices ();
    void configureTimeService ();
    void configureSLURM (const std::unique_ptr<Cluster>&);
    void configureInfiniband ();
    void disableNetworkManagerDNSOverride (); // This should be on Network
    void configureInternalNetwork (); // This should be on Network
    void configureNetworkFileSystem ();

public:
    Shell();
    ~Shell();

    void testInstall(const std::unique_ptr<Cluster>&) override;
    void install();
};

#endif //CLOYSTER_SHELL_H
