//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 6/19/23.
//

#ifndef CLOYSTERHPC_NFS_H_
#define CLOYSTERHPC_NFS_H_

#include "services/IService.h"
#include <boost/asio.hpp>
#include <string>

class NFS : IService {
private:
    std::string m_directoryName;
    std::string m_directoryPath;
    std::string m_permissions;
    std::string m_fullPath;
    boost::asio::ip::address m_address;

public:
    NFS(const std::string& directoryName, const std::string& directoryPath,
        const boost::asio::ip::address& address);
    NFS(const std::string& directoryName, const std::string& directoryPath,
        const boost::asio::ip::address& address,
        const std::string& permissions);
    void configure();
    void enable();
    void disable();
    void start();
    void stop();

private:
    void setFullPath();
};

#endif // CLOYSTERHPC_NFS_H_
