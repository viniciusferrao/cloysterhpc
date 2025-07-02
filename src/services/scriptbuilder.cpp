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

TEST_SUITE_BEGIN("cloyster::services::ScriptBuilder");

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
    return
        addCommand("# Removing line with {} from {}", key, path)
        .addCommand(
            "grep -q {} {} && sed -i /{}/d {}",
            key, path, key, path);
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
        .addCommand("# Foo")
        .addCommand("foo")
        .addNewLine()
        .addLineToFile(
            "/etc/hosts", 
            "example.com",
            "123.123.123.123 example.com", 10)
        .enableService("foo-service")
        ;
    CHECK(builder.toString() == 
R"del(#!/bin/bash -xeu

# Foo
foo

grep -q "example.com" "/etc/hosts" || \
  echo "123.123.123.123 example.com" >> "/etc/hosts"
systemctl enable --now foo-service)del");
}
// windsurf https://aider.chat/
// codex (ai) codex-cli
// e como adicionar um CI

TEST_SUITE_END();

}; // namespace cloyster::services
