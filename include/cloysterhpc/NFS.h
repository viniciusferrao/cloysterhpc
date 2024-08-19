/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NFS_H_
#define CLOYSTERHPC_NFS_H_

#include <boost/asio.hpp>
#include <cloysterhpc/services/IService.h>
#include <string>

/**
 * @class NFS
 * @brief A class representing the NFS service configuration.
 *
 * This class provides methods for configuring, enabling, disabling, starting,
 * and stopping the NFS service.
 */
class NFS : public IService {
private:
    std::string m_directoryName;
    std::string m_directoryPath;
    std::string m_permissions;
    std::string m_fullPath;
    boost::asio::ip::address m_address;

public:
    /**
     * @brief Constructs an NFS object with the specified parameters.
     *
     * @param directoryName The name of the directory to be shared via NFS.
     * @param directoryPath The path of the directory to be shared via NFS.
     * @param address The IP address of the NFS server.
     * @param permissions The permissions for the NFS share.
     */
    NFS(const std::string& directoryName, const std::string& directoryPath,
        const boost::asio::ip::address& address,
        const std::string& permissions);

    /**
     * @brief Configures the NFS service with the specified settings.
     */
    void configure();

    /**
     * @brief Enables the NFS service.
     */
    void enable() final;

    /**
     * @brief Disables the NFS service.
     */
    void disable() final;

    /**
     * @brief Starts the NFS service.
     */
    void start() final;

    /**
     * @brief Stops the NFS service.
     */
    void stop() final;

private:
    /**
     * @brief Sets the full path of the NFS share.
     */
    void setFullPath();
};

#endif // CLOYSTERHPC_NFS_H_
