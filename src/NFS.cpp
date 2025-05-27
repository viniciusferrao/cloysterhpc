/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fmt/format.h>

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/utils/formatters.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/scriptbuilder.h>

using cloyster::models::OS;

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::services {
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

cloyster::services::ScriptBuilder
NFS::installScript(const OS& osinfo) 
{
    using namespace cloyster;
    services::ScriptBuilder builder(osinfo);
    builder
        .addNewLine()
        .addCommand("# Variables")
        .addCommand("HEADNODE=$(hostname -s)")
        .addNewLine()
        .addCommand("# install packages")
        .addPackage("nfs-utils autofs")
        .addNewLine()
        .addCommand("# Add exports to /etc/exports")
        .addLineToFile(
            "/etc/exports", 
            "/home",
            "/home *(rw,no_subtree_check,fsid={},no_root_squash)", 10)
        .addLineToFile(
            "/etc/exports", 
            "/opt/ohpc/pub",
            "/opt/ohpc/pub *(ro,no_subtree_check,fsid={})", 11)
        .addLineToFile(
            "/etc/exports", 
            "/tftpboot",
            "/tftpboot *(rw,no_root_squash,sync,no_subtree_check)")
        .addLineToFile(
            "/etc/exports", 
            "/install",
            "/install *(rw,no_root_squash,sync,no_subtree_check)")
        .addNewLine()
        .enableService("rpcbind nfs-server autofs")
        .addCommand("exportfs -a")
        .addNewLine()
        .addCommand(R"(# Update firewall rules
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --permanent --add-service={{nfs,mountd,rpc-bind}}
    firewall-cmd --reload
fi)");
    return builder;
}

cloyster::services::ScriptBuilder NFS::imageInstallScript(
    const OS& osinfo,
    const cloyster::services::XCAT::ImageInstallArgs& args)
{
    using namespace cloyster;
    services::ScriptBuilder builder(osinfo);

    builder.addNewLine()
        .addCommand("# Define variables (for shell script execution)")
        .addCommand("IMAGE=\"{}\"", args.imageName)
        .addCommand("ROOTFS=\"{}\"", args.rootfs)
        .addCommand("POSTINSTALL=\"{}\"", args.postinstall)
        .addCommand("PKGLIST=\"{}\"", args.pkglist)
        .addCommand("HEADNODE=$(hostname -s)")
        .addNewLine()
        .addCommand("# Add autofs commands to postinstall")
        .addLineToFile("${POSTINSTALL}", "autofs", "systemctl enable --now autofs")
        .addCommand("chmod +x \"${{POSTINSTALL}}\"")
        .addNewLine()
        .addCommand("# Add required packages to the image")
        .addLineToFile(
            "${PKGLIST}",
            "nfs-utils",
            "nfs-utils")
        .addNewLine()
        .addCommand("# Configure autofs")
        .addLineToFile(
            "${ROOTFS}/etc/auto.master",
            "/home",
            "/home   /etc/auto.home")
        .addLineToFile(
            "${ROOTFS}/etc/auto.master",
            "/opt/ohpc/pub",
            "/opt/ohpc/pub   /etc/auto.ohpc")
        .addNewLine()
        .addLineToFile(
            "${ROOTFS}/etc/auto.master",
            ":/home/&",
            "* -fstype=nfs,rw,no_subtree_check,no_root_squash ${{HEADNODE}}:/home/&")
        .addNewLine()
        .addLineToFile(
            "${ROOTFS}/etc/auto.master",
            "/opt/ohpc/pub/&",
            "* -fstype=nfs,ro,no_subtree_check ${{HEADNODE}}:/opt/ohpc/pub/&")
        .addNewLine()
        .addCommand("# Create mount points")
        .addCommand("mkdir -p ${{ROOTFS}}/home ${{ROOTFS}}/opt/ohpc/pub || :")
        .addNewLine()
        .addCommand("# Update xCAT configuration")
        .addCommand(
            "chdef -t osimage ${{IMAGE}} postinstall=\"${{POSTINSTALL}}\"")
        .addNewLine();
    return builder;
}

TEST_SUITE_BEGIN("cloyster::services::NFS");

TEST_CASE("installScript") {
    const OS osinfo
        = cloyster::models::OS(OS::Distro::Rocky, OS::Platform::el9, 5);
    const auto builder = NFS::installScript(osinfo);
    
    // @TODO, this test check for string equality, while simple
    // it is very sensitive to changes, make this test more propositional
    // like:
    //
    // - Check if the packages are being installed
    // - Check if the files are being updated
    // - Check if packages are being installed
    // - etc...
    CHECK(builder.toString() == 
R"del(#!/bin/bash -xeu

# Variables
HEADNODE=$(hostname -s)

# install packages
dnf install -y nfs-utils autofs

# Add exports to /etc/exports
grep -q "/home" || \
  echo "/home *(rw,no_subtree_check,fsid=10,no_root_squash)" >> "/etc/exports"
grep -q "/opt/ohpc/pub" || \
  echo "/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)" >> "/etc/exports"
grep -q "/tftpboot" || \
  echo "/tftpboot *(rw,no_root_squash,sync,no_subtree_check)" >> "/etc/exports"
grep -q "/install" || \
  echo "/install *(rw,no_root_squash,sync,no_subtree_check)" >> "/etc/exports"

systemctl enable --now rpcbind nfs-server autofs
exportfs -a

# Update firewall rules
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --permanent --add-service={nfs,mountd,rpc-bind}
    firewall-cmd --reload
fi)del");
}

TEST_CASE("installImageScript") {
    const OS osinfo
        = cloyster::models::OS(OS::Distro::Rocky, OS::Platform::el9, 5);
    const auto builder = NFS::imageInstallScript(osinfo, {
        .imageName = "rocky9.5-x86_64-netboot-compute",
        .rootfs = "/install/netboot/rocky9.5/x86_64/compute/rootimg",
        .postinstall = "/install/custom/netboot/compute.postinstall",
        .pkglist = "/install/custom/netboot/compute.otherpkglist"
    });
    
    CHECK(builder.toString() == 
R"del(#!/bin/bash -xeu

# Define variables (for shell script execution)
IMAGE="rocky9.5-x86_64-netboot-compute"
ROOTFS="/install/netboot/rocky9.5/x86_64/compute/rootimg"
POSTINSTALL="/install/custom/netboot/compute.postinstall"
PKGLIST="/install/custom/netboot/compute.otherpkglist"
HEADNODE=$(hostname -s)

# Add autofs commands to postinstall
grep -q "autofs" || \
  echo "systemctl enable --now autofs" >> "${POSTINSTALL}"
chmod +x "${POSTINSTALL}"

# Add required packages to the image
grep -q "nfs-utils" || \
  echo "nfs-utils" >> "${PKGLIST}"

# Configure autofs
grep -q "/home" || \
  echo "/home   /etc/auto.home" >> "${ROOTFS}/etc/auto.master"
grep -q "/opt/ohpc/pub" || \
  echo "/opt/ohpc/pub   /etc/auto.ohpc" >> "${ROOTFS}/etc/auto.master"

grep -q ":/home/&" || \
  echo "* -fstype=nfs,rw,no_subtree_check,no_root_squash ${HEADNODE}:/home/&" >> "${ROOTFS}/etc/auto.master"

grep -q "/opt/ohpc/pub/&" || \
  echo "* -fstype=nfs,ro,no_subtree_check ${HEADNODE}:/opt/ohpc/pub/&" >> "${ROOTFS}/etc/auto.master"

# Create mount points
mkdir -p ${ROOTFS}/home ${ROOTFS}/opt/ohpc/pub || :

# Update xCAT configuration
chdef -t osimage ${IMAGE} postinstall="${POSTINSTALL}"
)del");
}
};

TEST_SUITE_END();
