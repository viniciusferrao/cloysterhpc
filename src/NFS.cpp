/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fmt/format.h>

#include <cloysterhpc/NFS.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/scriptbuilder.h>
#include <cloysterhpc/utils/formatters.h>
#include <cloysterhpc/utils/singleton.h>
#include <string_view>

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

cloyster::services::ScriptBuilder NFS::installScript(const OS& osinfo)
{
    using namespace cloyster;
    services::ScriptBuilder builder(osinfo);
    builder.addNewLine()
        .addCommand("# Variables")
        .addCommand("HEADNODE=$(hostname -s)")
        .addNewLine()
        .addCommand("# install packages")
        .addPackage("nfs-utils")
        .addNewLine()
        .addCommand("# Add exports to /etc/exports")
        .addLineToFile("/etc/exports", "/home",
            "/home *(rw,no_subtree_check,fsid={},no_root_squash)", 10)
        .addLineToFile("/etc/exports", "/opt/ohpc/pub",
            "/opt/ohpc/pub *(ro,no_subtree_check,fsid={})", 11)
        .addLineToFile("/etc/exports", "/opt/spack",
            "/opt/spack *(ro)");
    if (utils::singleton::answerfile()->system.provisioner == "xcat") {
        builder
            .addLineToFile("/etc/exports", "/tftpboot",
                           "/tftpboot *(rw,no_root_squash,sync,no_subtree_check)")
            .addLineToFile("/etc/exports", "/install",
                           "/install *(rw,no_root_squash,sync,no_subtree_check)")
            .addNewLine();
    }
    builder
        .enableService("rpcbind nfs-server")
        .addCommand("exportfs -a > /dev/null 2>&1 || :")
        .addNewLine()
        .addCommand(R"(# Update firewall rules
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --permanent --add-service={{nfs,mountd,rpc-bind}}
    firewall-cmd --reload
fi)");
    return builder;
}

cloyster::services::ScriptBuilder NFS::imageInstallScript(
    const OS& osinfo, const cloyster::services::XCAT::ImageInstallArgs& args)
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
        .addLineToFile("${POSTINSTALL}", "autofs",
            "chroot \\${{IMG_ROOTIMGDIR}} systemctl enable autofs")
        .addCommand("chmod +x \"${{POSTINSTALL}}\"")
        .addNewLine()
        .addCommand("# Add required packages to the image")
        .addLineToFile("${PKGLIST}", "nfs-utils", "nfs-utils")
        .addLineToFile("${PKGLIST}", "autofs", "autofs")
        .addNewLine()
        .addCommand("# Configure autofs")
        .addLineToFile(
            "${ROOTFS}/etc/auto.master", "/home", "/home   /etc/auto.home")
        .addLineToFile("${ROOTFS}/etc/auto.master", "/opt/ohpc/pub",
            "/opt/ohpc/pub   /etc/auto.ohpc")
        .addNewLine()
        .addLineToFile("${ROOTFS}/etc/auto.home", "home-map",
            "* -fstype=nfs,rw,no_subtree_check,no_root_squash "
            "${{HEADNODE}}:/home/&")
        .addNewLine()
        .addLineToFile("${ROOTFS}/etc/auto.ohpc", "ohpc-map",
            "* -fstype=nfs,ro,no_subtree_check ${{HEADNODE}}:/opt/ohpc/pub/&")
        .addNewLine()
        .addCommand("# Create mount points")
        .addCommand("mkdir -p ${{ROOTFS}}/home ${{ROOTFS}}/opt/ohpc/pub || :")
        .addNewLine()
        .addCommand("# Update xCAT configuration")
        .addCommand(
            "chdef -t osimage ${{IMAGE}} postinstall=\"${{POSTINSTALL}}\"");
    return builder;
}

TEST_SUITE_BEGIN("cloyster::services::NFS");

TEST_CASE("installScript")
{
    const OS osinfo
        = cloyster::models::OS(OS::Distro::Rocky, OS::Platform::el9, 5);
    const auto builder = NFS::installScript(osinfo);
    const auto scriptStr = builder.toString();
    const auto script = std::string_view(scriptStr);
    CHECK(script.contains("dnf install -y nfs-utils\n"));
    CHECK(script.contains("systemctl enable --now rpcbind nfs-server"));
    CHECK(script.contains("exportfs -a"));
    CHECK(script.contains("/home *(rw,no_subtree_check,fsid=10,no_root_squash)"));
    CHECK(script.contains("/opt/ohpc/pub *(ro,no_subtree_check,fsid=11)"));
    CHECK(script.contains("/tftpboot *(rw,no_root_squash,sync,no_subtree_check)"));
    CHECK(script.contains("/install *(rw,no_root_squash,sync,no_subtree_check)"));
}

TEST_CASE("installImageScript")
{
    const OS osinfo
        = cloyster::models::OS(OS::Distro::Rocky, OS::Platform::el9, 5);
    const auto builder = NFS::imageInstallScript(osinfo,
        { .imageName = "rocky9.5-x86_64-netboot-compute",
            .rootfs = "/install/netboot/rocky9.5/x86_64/compute/rootimg",
            .postinstall = "/install/custom/netboot/compute.postinstall",
            .pkglist = "/install/custom/netboot/compute.otherpkglist" });
    const std::string script = builder.toString();
    CHECK(script.contains("HEADNODE="));
    CHECK(script.contains("ROOTFS="));
    CHECK(script.contains("PKGLIST="));
    CHECK(script.contains("systemctl enable autofs"));
    CHECK(script.contains(R"(echo "nfs-utils" >> "${PKGLIST}")"));
    CHECK(script.contains(R"(echo "autofs" >> "${PKGLIST}")"));
    CHECK(script.contains(R"(echo "/home   /etc/auto.home" >> "${ROOTFS}/etc/auto.master")"));
    CHECK(script.contains(R"(echo "/opt/ohpc/pub   /etc/auto.ohpc" >> "${ROOTFS}/etc/auto.master")"));
    CHECK(script.contains(R"(echo "* -fstype=nfs,rw,no_subtree_check,no_root_squash ${HEADNODE}:/home/&" >> "${ROOTFS}/etc/auto.home")"));
    CHECK(script.contains(R"(echo "* -fstype=nfs,ro,no_subtree_check ${HEADNODE}:/opt/ohpc/pub/&" >> "${ROOTFS}/etc/auto.ohpc")"));
    CHECK(script.contains(R"(chdef -t osimage ${IMAGE} postinstall="${POSTINSTALL}")"));
};

}

TEST_SUITE_END();
