#include <filesystem>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/utils/formatters.h>

#include <cloysterhpc/services/scriptbuilder.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::services {

TEST_SUITE_BEGIN("ScriptBuilder");

using namespace cloyster::models;

ScriptBuilder::ScriptBuilder(const OS& osinfo)
    : m_os(osinfo)
{
    m_commands.emplace_back("#!/bin/bash -xeu");
};

ScriptBuilder& ScriptBuilder::addNewLine()
{
    return addCommand("");
}

ScriptBuilder& ScriptBuilder::enableService(const std::string_view service)
{
    return addCommand("systemctl enable --now {}", service);
}

ScriptBuilder& ScriptBuilder::disableService(const std::string_view service)
{
    return addCommand("systemctl disable --now {}", service);
};

ScriptBuilder& ScriptBuilder::startService(const std::string_view service)
{
    return addCommand("systemctl start {}", service);
}

ScriptBuilder& ScriptBuilder::stopService(const std::string_view service)
{
    return addCommand("systemctl stop {}", service);
};

ScriptBuilder& ScriptBuilder::addPackage(const std::string_view pkg)
{
    return addCommand("dnf install -y {}", pkg);
};

ScriptBuilder& ScriptBuilder::removePackage(const std::string_view pkg)
{
    return addCommand("dnf remove -y {}", pkg);
}

ScriptBuilder& ScriptBuilder::removeLineWithKeyFromFile(
    const std::filesystem::path& path,
    const std::string& key)
{
    return addCommand(
        "grep -q {} && sed -i /{}/d {}",
        key, key, path);
}

[[nodiscard]] std::string ScriptBuilder::toString() const
{
    return boost::algorithm::join(m_commands, "\n");
};

[[nodiscard]] const std::vector<std::string>& ScriptBuilder::commands() const
{
    return m_commands;
}

TEST_CASE("Basic") { 
    const OS osinfo = cloyster::models::OS(
        OS::Distro::Rocky,
        OS::Platform::el9,
        5
    );
    ScriptBuilder builder(osinfo); 

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
        .addCommand("# Configure autofs maps")
        .addLineToFile(
            "/etc/auto.home", 
            "/home",
            "/home   /etc/auto.home")
        .addLineToFile(
            "/etc/auto.home", 
            "/opt/ohpc/pub",
            "/opt/ohpc/pub   /etc/auto.home")
        .addLineToFile(
            "/etc/auto.home",
            ":/home/&",
            "* -fstype=nfs,rw,no_subtree_check,no_root_squash ${{HEADNODE}}:/home/&")
        .addLineToFile(
            "/etc/auto.home",
            ":/opt/ohpc/pub/&",
            "* -fstype=nfs,ro,no_subtree_check ${{HEADNODE}}:/opt/ohpc/pub/&")
        .enableService("rpcbind nfs-server autofs")
        .addCommand("exports -a")
        .addNewLine()
        .addCommand(R"(# Update firewall rules
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --permanent --add-service={{nfs,mountd,rpc-bind}}
    firewall-cmd --reload
fi)")
        ;
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

# Configure autofs maps
grep -q "/home" || \
  echo "/home   /etc/auto.home" >> "/etc/auto.home"
grep -q "/opt/ohpc/pub" || \
  echo "/opt/ohpc/pub   /etc/auto.home" >> "/etc/auto.home"
grep -q ":/home/&" || \
  echo "* -fstype=nfs,rw,no_subtree_check,no_root_squash ${HEADNODE}:/home/&" >> "/etc/auto.home"
grep -q ":/opt/ohpc/pub/&" || \
  echo "* -fstype=nfs,ro,no_subtree_check ${HEADNODE}:/opt/ohpc/pub/&" >> "/etc/auto.home"
systemctl enable --now rpcbind nfs-server autofs
exports -a

# Update firewall rules
if systemctl is-enabled --quiet firewalld.service; then
    firewall-cmd --permanent --add-service={nfs,mountd,rpc-bind}
    firewall-cmd --reload
fi)del");
}
// windsurf https://aider.chat/
// codex (ai) codex-cli
// e como adicionar um CI

TEST_SUITE_END();

}; // namespace cloyster::services
