/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <fmt/format.h>

using cloyster::runCommand;

NFS::NFS(std::shared_ptr<MessageBus> bus, const std::string& directoryName,
    const std::string& directoryPath, const boost::asio::ip::address& address,
    const std::string& permissions)
    : IService(bus, "nfs-server.service")
    , m_directoryName(directoryName)
    , m_directoryPath(directoryPath)
    , m_permissions(permissions)
    , m_address(address)
{
    setFullPath();
}

void NFS::setFullPath()
{
    m_fullPath = fmt::format("{}/{}", m_directoryPath, m_directoryName);
}

void NFS::configure()
{
    LOG_INFO("Configuring NFS");

    runCommand("dnf -y install nfs-utils");

    // TODO: detect nfs existence before proceeding
    // might check if nfs-utils package is installed.

    const std::string_view filename = CHROOT "/etc/exports";
    cloyster::backupFile(filename);
    cloyster::addStringToFile(filename,
        // @TODO make fsid dynamic
        fmt::format("/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
                    "{} *({},fsid={})\n",
            m_fullPath, m_permissions, 11));

    runCommand("exportfs -a");

    cloyster::touchFile(fmt::format("{}/conf/node/etc/auto.master.d/{}.autofs",
        installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.master.d/{}.autofs", installPath,
            m_directoryName),
        fmt::format("{} /etc/auto.{}", m_fullPath, m_directoryName));

    cloyster::touchFile(
        fmt::format("{}/conf/node/etc/auto.{}", installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.{}", installPath, m_directoryName),
        fmt::format("* {}:{}/&", m_address.to_string(), m_fullPath));
}
