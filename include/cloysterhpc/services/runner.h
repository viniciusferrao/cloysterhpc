/*
 * Copyright 2024 Arthur Mendes <arthur@versatushpc.com.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_RUNNER_H_
#define CLOYSTERHPC_RUNNER_H_

#include <boost/process.hpp>
#include <fmt/format.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/scriptbuilder.h>

namespace cloyster::services::runner::shell {

namespace unsafe {
    template <typename... Args>
    [[nodiscard]]
    int fmt(std::vector<std::string>& output,
        fmt::format_string<Args...> format, Args&&... args)
    {
        auto command = fmt::format(format, std::forward<Args>(args)...);

        auto opts = cloyster::Singleton<cloyster::services::Options>::get();
        if (!opts->dryRun) {
            LOG_DEBUG("Running shell command: {}", command);
            boost::process::ipstream pipe_stream;
            boost::process::child child("/bin/bash", "-c", command,
                boost::process::std_out > pipe_stream);

            std::string line;
            while (pipe_stream && std::getline(pipe_stream, line)) {
                output.emplace_back(line);
                LOG_TRACE("{}", line);
            }

            child.wait();
            LOG_DEBUG("Exit code: {}", child.exit_code());
            return child.exit_code();
        } else {
            LOG_INFO("Dry Run: {}", command);
            return 0;
        }
    }

    template <typename... Args>
    [[nodiscard]]
    int fmt(fmt::format_string<Args...> format, Args&&... args)
    {
        auto command = fmt::format(format, std::forward<Args>(args)...);

        auto opts = cloyster::Singleton<cloyster::services::Options>::get();
        if (!opts->dryRun) {
            LOG_DEBUG("Running shell command: {}", command);
            boost::process::ipstream pipe_stream;
            boost::process::child child("/bin/bash", "-c", command,
                boost::process::std_out > pipe_stream);

            std::string line;
            while (pipe_stream && std::getline(pipe_stream, line)) {
                LOG_TRACE("{}", line);
            }

            child.wait();
            LOG_DEBUG("Exit code: {}", child.exit_code());
            return child.exit_code();
        } else {
            LOG_INFO("Dry Run: {}", command);
            return 0;
        }
    }
}

template <typename... Args>
void fmt(fmt::format_string<Args...> format, Args&&... args)
{
    const std::string command
        = fmt::format(format, std::forward<Args>(args)...);
    const auto exitCode = unsafe::fmt("{}", command);
    if (exitCode != 0) {
        throw std::runtime_error(fmt::format(
            "Command {} failed with exit code {}", command, exitCode));
    }
}

void cmd(std::string_view cmd);

template <typename... Args>
[[nodiscard]]
std::string output(fmt::format_string<Args...> format, Args&&... args)
{
    std::vector<std::string> output;
    // Cosntruct a command here because it will be used twice and
    // we can't use args twice without hurting the perfect forwarding
    const std::string command
        = fmt::format(format, std::forward<Args>(args)...);
    const auto exitCode = unsafe::fmt(output, "{}", command);
    if (exitCode != 0) {
        throw std::runtime_error(fmt::format(
            "Command {} failed with exit code {}", command, exitCode));
    }
    return fmt::format("{}", fmt::join(output, "\n"));
}

}

namespace cloyster::services {

/**
 * @struct CommandProxy
 * @brief A command proxy to capture the command output while the command is
 * running.
 *
 * This structure is used to capture the output of a command in real-time,
 * useful for displaying progress in a dialog.
 */
struct CommandProxy {
    bool valid = false;
    boost::process::child child;
    boost::process::ipstream pipe_stream;

    /**
     * @brief Gets a line of output from the command.
     *
     * @return An optional string containing a line of output if available,
     * otherwise std::nullopt.
     */
    std::optional<std::string> getline();
    std::optional<std::string> getUntil(char chr);
};

enum class Stream : std::uint8_t { Stdout, Stderr };

/**
 * Works as an abstraction for command execution.
 */
class IRunner {
public:
    IRunner() = default;
    IRunner(const IRunner&) = default;
    IRunner(IRunner&&) = delete;
    IRunner& operator=(const IRunner&) = default;
    IRunner& operator=(IRunner&&) = delete;
    virtual ~IRunner() = default;

    virtual int executeCommand(const std::string&) = 0;
    virtual int executeCommand(
        const std::string&, std::list<std::string>& output)
        = 0;
    virtual CommandProxy executeCommandIter(
        const std::string&, Stream out = Stream::Stdout)
        = 0;
    virtual void checkCommand(const std::string&) = 0;
    virtual std::vector<std::string> checkOutput(const std::string&) = 0;
    virtual int downloadFile(const std::string& url, const std::string& file)
        = 0;

    virtual int run(const ScriptBuilder& script) = 0;
};

class Runner final : public IRunner {
public:
    int executeCommand(const std::string& cmd) override;
    int executeCommand(
        const std::string&, std::list<std::string>& output) override;
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;
    int run(const ScriptBuilder& script) override;
};

class DryRunner final : public IRunner {
public:
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    int executeCommand(const std::string& cmd) override;
    int executeCommand(
        const std::string&, std::list<std::string>& output) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;
    int run(const ScriptBuilder& script) override;
};

class MockRunner final : public IRunner {
public:
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    int executeCommand(const std::string& cmd) override;
    int executeCommand(
        const std::string&, std::list<std::string>& output) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;
    int run(const ScriptBuilder& script) override;

    [[nodiscard]] const std::vector<std::string>& listCommands() const;

private:
    std::vector<std::string> m_cmds;
};

} // namespace cloyster::services

#endif // CLOYSTERHPC_RUNNER_H_
