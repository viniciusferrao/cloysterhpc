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
        .addCommand("HEADNODE=$(hostname -s)")
        .addNewLine()
        .addCommand("# install packages")
        .addPackage("nfs-utils autofs")
        .addNewLine()
        .addCommand("# Add exports to /etc/exports")
        .addLineToFile(
            "/etc/exports", 
            "/home",
            "/home *(rw,no_subtree_check,fsid={},no_root_squash)", 10);
    CHECK(builder.toString() == 
R"(#!/bin/bash -xeu

HEADNODE=$(hostname -s)

# install packages
dnf install -y nfs-utils autofs

# Add exports to /etc/exports
grep -q /home || \
  echo /home *(rw,no_subtree_check,fsid=10,no_root_squash) >> /etc/exports)");
}

TEST_SUITE_END();

}; // namespace cloyster::services
