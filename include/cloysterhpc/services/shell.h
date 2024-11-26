/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_SHELL_H_
#define CLOYSTERHPC_SHELL_H_

#include <magic_enum.hpp>

#include <cloysterhpc/cluster.h>
#include <cloysterhpc/services/execution.h>
#include <cloysterhpc/services/provisioner.h>

/**
 * @class Shell
 * @brief Manages the configuration and installation processes on a cluster.
 *
 * This class provides functionalities for configuring various system settings,
 * installing required packages, and setting up cluster-specific services.
 */
class Shell : public Execution {
private:
    const std::unique_ptr<Cluster>& m_cluster;

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
    void disableNetworkManagerDNSOverride(); // This should be on Network

    void deleteConnectionIfExists(std::string_view connectionName);

    /**
     * @brief Configures network connections.
     *
     * This function sets up the network interfaces based on the provided
     * connections.
     *
     * @param connections A list of network connections to configure.
     */
    void configureNetworks(const std::list<Connection>&);

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
    void installRequiredPackages();

    /**
     * @brief Disallows SSH root password login.
     *
     * This function configures SSH to disallow root login using password
     * authentication.
     */
    void disallowSSHRootPasswordLogin();

    /**
     * @brief Installs the OpenHPC base packages.
     *
     * This function installs the base packages required for OpenHPC.
     */
    void installOpenHPCBase();

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
    void configureTimeService(const std::list<Connection>&);

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
    void removeMemlockLimits();
    /**
     * @brief Installs development components.
     *
     * This function installs the necessary development tools and libraries.
     */
    void installDevelopmentComponents();

    /* Ancillary functions */
    /**
     * @brief Disables SELinux.
     *
     * This function completely disables SELinux enforcement.
     */
    void disableSELinux();

public:
    // FIXME: Guideline: Don’t use a const unique_ptr& as a parameter;
    //  use widget* instead.
    /**
     * @brief Constructs a Shell object.
     *
     * Initializes the Shell object with a reference to a Cluster object.
     *
     * @param cluster A reference to a unique pointer managing a Cluster object.
     */
    explicit Shell(const std::unique_ptr<Cluster>& cluster);
    /**
     * @brief Installs and configures the system.
     *
     * This function performs the installation and configuration processes.
     */
    void install() override;
};

#endif // CLOYSTERHPC_SHELL_H_
