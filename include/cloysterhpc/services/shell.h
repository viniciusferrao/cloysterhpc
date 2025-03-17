/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SHELL_H_
#define CLOYSTERHPC_SHELL_H_

#include <magic_enum/magic_enum.hpp>

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
 */
class Shell final : public Execution {
private:

    /**
     * @brief Configures SELinux mode.
     *
     * This function sets the appropriate SELinux mode for the system.
     */
    void configureSELinuxMode();

    /**
     * @brief Configures the system firewall.
     *
     * This function sets up the necessary firewall rules and policies.
     */
    void configureFirewall();

    /**
     * @brief Configures the Fully Qualified Domain Name (FQDN).
     *
     * This function sets the system's FQDN.
     */
    void configureFQDN();

    /**
     * @brief Configures the /etc/hosts file.
     *
     * This function updates the /etc/hosts file with necessary entries.
     */
    void configureHostsFile();

    /**
     * @brief Configures the system timezone.
     *
     * This function sets the system's timezone.
     */
    void configureTimezone();

    /**
     * @brief Configures the system locale.
     *
     * This function sets the system's locale settings.
     */
    void configureLocale();

    /**
     * @brief Disables NetworkManager DNS override.
     *
     * This function disables the DNS override feature of NetworkManager.
     */
    static void disableNetworkManagerDNSOverride(); // This should be on Network

    static void deleteConnectionIfExists(std::string_view connectionName);

    /**
     * @brief Configures network connections.
     *
     * This function sets up the network interfaces based on the provided
     * connections.
     *
     * @param connections A list of network connections to configure.
     */
    static void configureNetworks(const std::list<Connection>&);

    /**
     * @brief Runs a system update.
     *
     * This function updates the system packages to their latest versions.
     */
    void runSystemUpdate();

    /**
     * @brief Installs required packages.
     *
     * This function installs all the necessary packages for the system to
     * operate.
     */
    static void installRequiredPackages();

    /**
     * @brief Disallows SSH root password login.
     *
     * This function configures SSH to disallow root login using password
     * authentication.
     */
    static void disallowSSHRootPasswordLogin();

    /**
     * @brief Installs the OpenHPC base packages.
     *
     * This function installs the base packages required for OpenHPC.
     */
    static void installOpenHPCBase();

    /**
     * @brief Configure repositories
     *
     * This function configure the required repos
     */
    void configureRepositories();

    /**
     * @brief Configures the time service.
     *
     * This function sets up the time synchronization service using the provided
     * connections.
     *
     * @param connections A list of network connections to use for time service
     * configuration.
     */
    static void configureTimeService(const std::list<Connection>&);

    /**
     * @brief Configures the queue system.
     *
     * This function sets up the queue system for managing jobs in the cluster.
     */
    void configureQueueSystem();

    /**
     * @brief Configures the InfiniBand settings.
     *
     * This function sets up the InfiniBand interconnect settings.
     */
    void configureInfiniband();
    void configureMailSystem();

    /**
     * @brief Removes memory lock limits.
     *
     * This function removes any limits on memory locking.
     */
    static void removeMemlockLimits();
    /**
     * @brief Installs development components.
     *
     * This function installs the necessary development tools and libraries.
     */
    static void installDevelopmentComponents();

    /* Ancillary functions */
    /**
     * @brief Disables SELinux.
     *
     * This function completely disables SELinux enforcement.
     */
    static void disableSELinux();

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
