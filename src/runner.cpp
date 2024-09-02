#include <cloysterhpc/functions.h>
#include <cloysterhpc/runner.h>
#include <cloysterhpc/services/log.h>
#include <fmt/format.h>

int BaseRunner::downloadFile(const std::string& url, const std::string& file)
{
    auto cmd = fmt::format("wget -NP {} {}", file, url);
    return this->executeCommand(cmd);
}

int Runner::executeCommand(const std::string& cmd)
{
    return cloyster::runCommand(cmd);
}

int DryRunner::executeCommand(const std::string& cmd)
{
    LOG_INFO("Would execute command: {}", cmd);
    return 0;
}

int MockRunner::executeCommand(const std::string& cmd)
{
    m_cmds.push_back(cmd);
    return 0;
}

const std::vector<std::string>& MockRunner::listCommands() const
{
    return m_cmds;
}
