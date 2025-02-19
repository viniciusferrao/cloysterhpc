#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

#include <fmt/format.h>

namespace cloyster::services {

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
    LOG_WARN("Dry Run: Would execute command: {}", cmd);
    return OK;
}

int MockRunner::executeCommand(const std::string& cmd)
{
    m_cmds.push_back(cmd);
    return OK;
}

const std::vector<std::string>& MockRunner::listCommands() const
{
    return m_cmds;
}

} // namespace cloyster::services
