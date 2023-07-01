/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "NFS.h"
#include "const.h"
#include "functions.h"
#include <fmt/format.h>

using cloyster::runCommand;

// @TODO: Constructors should be chained and m_permissions should be an enum
NFS::NFS(const std::string& directoryName, const std::string& directoryPath,
    const boost::asio::ip::address& address)
    : m_directoryName(directoryName)
    , m_directoryPath(directoryPath)
    , m_address(address)
{
    setFullPath();
    m_permissions = "ro";
}

NFS::NFS(const std::string& directoryName, const std::string& directoryPath,
    const boost::asio::ip::address& address, const std::string& permissions)
    : m_directoryName(directoryName)
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
    std::string_view filename = CHROOT "/etc/exports";
    cloyster::backupFile(filename);
    cloyster::addStringToFile(filename,
        // @TODO make fsid dynamic
        fmt::format("/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
                    "{} *({},fsid={})\n", m_fullPath, m_permissions, 11));
    
    runCommand("exportfs -a");

    // @FIXME: Create a file using std::filesystem and not with touch
    runCommand(fmt::format("touch {}/conf/node/etc/auto.master.d/{}.autofs",
        installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.master.d/{}.autofs", installPath,
            m_directoryName),
        fmt::format("{} /etc/auto.{}", m_fullPath, m_directoryName));

    // @FIXME: Create a file using std::filesystem and not with touch
    runCommand(fmt::format(
        "touch {}/conf/node/etc/auto.{}", installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.{}", installPath, m_directoryName),
        fmt::format("* {}:{}/&", m_address.to_string(), m_fullPath));
}

void NFS::enable() { runCommand("systemctl enable nfs-server"); }

void NFS::disable() { runCommand("systemctl disable nfs-server"); }

void NFS::start() { runCommand("systemctl start nfs-server"); }

void NFS::stop() { runCommand("systemctl stop nfs-server"); }
