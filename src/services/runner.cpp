#include <cloysterhpc/const.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

#include <fmt/format.h>
#include <ranges>

using cloyster::services::CommandProxy;
using cloyster::services::Stream;

namespace {

std::tuple<bool, std::optional<std::string>> retrieveLine(
    boost::process::ipstream& pipe_stream,
    const std::function<std::string(boost::process::ipstream&)>& linecheck)
{
    if (pipe_stream.good()) {
        return make_tuple(true, make_optional(linecheck(pipe_stream)));
    }

    return make_tuple(pipe_stream.good(), std::nullopt);
}

CommandProxy runCommandIter(
    const std::string& command, Stream out, bool overrideDryRun)
{
    if (!cloyster::dryRun || overrideDryRun) {
        LOG_DEBUG("Running interative command: {}", command)
        boost::process::ipstream pipe_stream;

        if (out == Stream::Stderr) {
            boost::process::child child(
                command, boost::process::std_err > pipe_stream);
            return CommandProxy { .valid = true,
                .child = std::move(child),
                .pipe_stream = std::move(pipe_stream) };

        } else {
            boost::process::child child(
                command, boost::process::std_out > pipe_stream);
            return CommandProxy { .valid = true,
                .child = std::move(child),
                .pipe_stream = std::move(pipe_stream) };
        }
    }

    return CommandProxy {};
}

int runCommand(const std::string& command, std::list<std::string>& output,
    bool overrideDryRun)
{

    if (!cloyster::dryRun || overrideDryRun) {
        LOG_DEBUG("Running command: {}", command)
        boost::process::ipstream pipe_stream;
        boost::process::child child(
            command, boost::process::std_out > pipe_stream);

        std::string line;

        while (pipe_stream && std::getline(pipe_stream, line)) {
            LOG_TRACE("{}", line)
            output.emplace_back(line);
        }

        child.wait();
        LOG_DEBUG("Exit code: {}", child.exit_code())
        return child.exit_code();
    } else {
        LOG_INFO("Dry Run: {}", command)
        return 0;
    }
}

int runCommand(const std::string& command, bool overrideDryRun)
{
    std::list<std::string> output;
    return runCommand(command, output, overrideDryRun);
}

}; // namespace {

namespace cloyster::services {

std::optional<std::string> CommandProxy::getline()
{

    if (!valid)
        return std::nullopt;

    auto [new_valid, out_line]
        = retrieveLine(pipe_stream, [this](boost::process::ipstream& pipe) {
              if (std::string line = ""; std::getline(pipe, line)) {
                  return line;
              }

              valid = false;
              return std::string {};
          });

    valid = new_valid;
    return out_line;
}

std::optional<std::string> CommandProxy::getUntil(char chr)
{
    if (!valid) {
        return std::nullopt;
    }

    auto [new_valid, out_line] = retrieveLine(
        pipe_stream, [this, chr](boost::process::ipstream& pipe) {
            if (std::string line; std::getline(pipe, line, chr)) {
                return line;
            }

            valid = false;
            return std::string {};
        });

    valid = new_valid;
    return out_line;
}

// Runner impl
int Runner::executeCommand(const std::string& cmd)
{
    return runCommand(cmd, true);
}

CommandProxy Runner::executeCommandIter(const std::string& cmd, Stream /*out*/)
{
    return CommandProxy {}; // Return an invalid CommandProxy
}

int Runner::downloadFile(const std::string& url, const std::string& file)
{
    auto cmd = fmt::format("wget -NP {} {}", file, url);
    return this->executeCommand(cmd);
}

void Runner::checkCommand(const std::string& cmd)
{
    if (runCommand(cmd, true) != 0) {
        throw std::runtime_error(
            fmt::format("ERROR: Command failed '{}'", cmd));
    }
}

std::vector<std::string> Runner::checkOutput(const std::string& cmd)
{
    std::list<std::string> output;
    if (runCommand(cmd, output, false) != 0) {
        throw std::runtime_error(
            fmt::format("ERROR: Command failed '{}'", cmd));
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

CommandProxy DryRunner::executeCommandIter(
    const std::string& cmd, Stream /*out*/)
{
    LOG_WARN("Dry Run: Would execute iterative command: {}", cmd);
    return CommandProxy {}; // Return an invalid CommandProxy
}

int DryRunner::downloadFile(const std::string& url, const std::string& file)
{
    LOG_WARN("Dry Run: Would download file from {} to {}", url, file);
    return OK;
}

int MockRunner::executeCommand(const std::string& cmd)
{
    m_cmds.push_back(cmd);
    return OK;
}

void MockRunner::checkCommand(const std::string& cmd) { }

std::vector<std::string> MockRunner::checkOutput(const std::string& /*cmd*/)
{
    return {};
}

const std::vector<std::string>& MockRunner::listCommands() const
{
    return m_cmds;
}

CommandProxy MockRunner::executeCommandIter(
    const std::string& cmd, Stream /*out*/)
{
    m_cmds.push_back(cmd);
    return CommandProxy {}; // Return an invalid CommandProxy
}

int MockRunner::downloadFile(const std::string& url, const std::string& file)
{
    auto cmd = fmt::format("wget -NP {} {}", file, url);
    m_cmds.push_back(cmd);
    return OK;
}

} // namespace cloyster::services
