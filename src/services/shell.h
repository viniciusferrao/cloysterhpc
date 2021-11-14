//
// Created by Vinícius Ferrão on 31/10/21.
//
#ifndef CLOYSTER_SHELL_H
#define CLOYSTER_SHELL_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include "execution.h"
#include "../cluster.h"
#include "provisioner.h"

/* Adding XCAT (Provisioner) as a friend class of Shell will block the usage of
 * the static function runCommand outside of Shell (and it's friends). The only
 * drawback is that we now enabled friend classes full access to contents of
 * Shell class, which may not be desirable.
 */
class Shell : public Execution {
    friend class XCAT;

private:
    static std::string m_commandOutput;

private:
    static int runCommand(const std::string&);
    void configureSELinuxMode (Cluster::SELinuxMode);
    void configureFirewall (bool);
    void configureFQDN (const std::string&);
    void configureHostsFile (std::string_view, std::string_view,
                             std::string_view);
    void configureTimezone (const std::string&);
    void configureLocale (const std::string&);

    void disableNetworkManagerDNSOverride (); // This should be on Network
    void configureNetworks(const std::unique_ptr<Cluster>&);

    void runSystemUpdate (bool);
    void installRequiredPackages ();

    void configureRepositories (const std::unique_ptr<Cluster>&);
    void installOpenHPCBase ();
    void configureTimeService ();
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
