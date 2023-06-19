//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 6/19/23.
//

#include "NFS.h"
#include "const.h"
#include "functions.h"
#include <fmt/format.h>

using cloyster::runCommand;

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
        fmt::format("/home *(rw,no_subtree_check,fsid=10,no_root_squash)\n"
                    "{} *({})\n",
            m_fullPath, m_permissions));

    runCommand("exportfs -a");
    runCommand("systemctl enable --now nfs-server");

    runCommand(fmt::format("touch {}/conf/node/etc/auto.master.d/{}.autofs",
        installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.master.d/{}.autofs", installPath,
            m_directoryName),
        fmt::format("{} /etc/auto.{}", m_fullPath, m_directoryName));

    runCommand(fmt::format(
        "touch {}/conf/node/etc/auto.{}", installPath, m_directoryName));
    cloyster::addStringToFile(
        fmt::format("{}/conf/node/etc/auto.{}", installPath, m_directoryName),
        fmt::format("* {0}:{1}/&", m_address.to_string(), m_fullPath));
}