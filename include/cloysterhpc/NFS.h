/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_NFS_H_
#define CLOYSTERHPC_NFS_H_

#include <boost/asio.hpp>
#include <cloysterhpc/services/IService.h>
#include <string>

class NFS : public IService {
private:
    std::string m_directoryName;
    std::string m_directoryPath;
    std::string m_permissions;
    std::string m_fullPath;
    boost::asio::ip::address m_address;

public:
    NFS(const std::string& directoryName, const std::string& directoryPath,
        const boost::asio::ip::address& address,
        const std::string& permissions);

    void install() final {};
    void configure() final;
    void enable() final;
    void disable() final;
    void start() final;
    void stop() final;

private:
    void setFullPath();
};

#endif // CLOYSTERHPC_NFS_H_
