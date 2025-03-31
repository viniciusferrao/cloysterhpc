#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

#include <fmt/format.h>
#include <ranges>

namespace cloyster::services {

int IRunner::downloadFile(const std::string& url, const std::string& file)
{
    auto cmd = fmt::format("wget -NP {} {}", file, url);
    return this->executeCommand(cmd);
}

int Runner::executeCommand(const std::string& cmd)
{
    return cloyster::runCommand(cmd);
}

void Runner::checkCommand(const std::string& cmd)
{
    if (cloyster::runCommand(cmd) != 0) {
        throw std::runtime_error(fmt::format("ERROR: Command failed '{}'", cmd));
    }
}

std::vector<std::string> Runner::checkOutput(const std::string& cmd)
{
    std::list<std::string> output;
    if (cloyster::runCommand(cmd, output) != 0) {
        throw std::runtime_error(fmt::format("ERROR: Command failed '{}'", cmd));
    }
    return output | std::ranges::to<std::vector>();
}


int DryRunner::executeCommand(const std::string& cmd)
{
    LOG_WARN("Dry Run: Would execute command: {}", cmd);
    return OK;
}

void DryRunner::checkCommand(const std::string& cmd)
{
    LOG_WARN("Dry Run: Would execute command: {}", cmd);
}

std::vector<std::string> DryRunner::checkOutput(const std::string& cmd)
{
    LOG_WARN("Dry Run: Would check output of command: {}", cmd);
    return {};
}

int MockRunner::executeCommand(const std::string& cmd)
{
    m_cmds.push_back(cmd);
    return OK;
}

void MockRunner::checkCommand(const std::string& cmd)
{
}

std::vector<std::string> MockRunner::checkOutput(const std::string& cmd)
{
    return {};
}

const std::vector<std::string>& MockRunner::listCommands() const
{
    return m_cmds;
}

} // namespace cloyster::services
