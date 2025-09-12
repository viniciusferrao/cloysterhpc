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

/**
 * @brief Hold functions to execute commands inside shell. This is
 * usefull to have pipes, file descriptor, execute commands that
 * depends on /etc/profile.d/ files, etc.
 *
 * WARNING: THESE FUNCTIONS ARE VULNERABLE TO SHELL INJECTION
 *
 * While this is a concern in genenral, in this case (this project) the user
 * need root access to run the project so ... but do not COPY/PASTE this to
 * projects that run behind a network or you'll introduce an attack vector.
 */
namespace cloyster::services::runner::shell {

/**
 * @brief Hold "unsafe" shell functions. Unsafe here means that
 * the caller is responsible for checking the exit code, the
 * execution is not aborted if the command fails.
 */
namespace unsafe {
    /**
     * @brief Executes a raw shell command and captures its output.
     *
     * Runs the provided command string in a new `/bin/bash -lc` process
     * and collects each line of output into the provided vector.
     *
     * The following bash options are used:
     * -l: load /etc/profile.d/ files
     * -x: enable bash debug
     * -e: stop in the first error (use command || : to mitigate)
     * -o pipefail: return the rightmost non-zero exit code in pipelines
     *
     * Behavior depends on runtime options:
     * - If `--dry-run` is enabled, the command is not executed, only logged.
     *   In this case, no output is collected and the return value is `0`.
     *
     * @param output A vector that will be populated with each line of
     *        command output (stdout/stderr).
     * @param command The raw shell command to execute.
     * @return The exit code of the executed command, or `0` in dry-run mode.
     */
    int cmd(std::vector<std::string>& output, std::string_view command);

    /**
     * @brief Executes a raw shell command without capturing output.
     *
     * Runs the provided command string in a new `/bin/bash -lc` process.
     * Output is streamed to the logging system only and not captured.
     *
     * The following bash options are used:
     * -l: load /etc/profile.d/ files
     * -x: enable bash debug
     * -e: stop in the first error (use command || : to mitigate)
     * -o pipefail: return the rightmost non-zero exit code in pipelines
     *
     * Behavior depends on runtime options:
     * - If `--dry-run` is enabled, the command is not executed, only logged.
     *   In this case, the return value is `0`.
     *
     * @param command The raw shell command to execute.
     * @return The exit code of the executed command, or `0` in dry-run mode.
     */
    int cmd(std::string_view command);

    /**
     * @brief Executes a formatted shell command and captures its output.
     *
     * This function runs a shell command constructed from a format string
     * and arguments. The command is executed in a new `/bin/bash` process
     * with `-lc` flags enabled:
     *
     * The following bash options are used:
     * -l: load /etc/profile.d/ files
     * -x: enable bash debug
     * -e: stop in the first error (use command || : to mitigate)
     * -o pipefail: return the rightmost non-zero exit code in pipelines
     *
     * Behavior depends on runtime options:
     * - If `--dry-run` is enabled, the command is not executed, only logged.
     *   are logged line-by-line for troubleshooting.
     *
     * @tparam Args Variadic template parameter pack for formatting arguments.
     * @param output A vector to store each line of command output.
     * @param format The fmtlib format string for constructing the command.
     * @param args Arguments to format into the command string.
     * @return The exit code of the executed command, or 0 in dry-run mode.
     */
    template <typename... Args>
    [[nodiscard]]
    int fmt(std::vector<std::string>& output,
        fmt::format_string<Args...> format, Args&&... args)
    {
        return cmd(output, fmt::format(format, std::forward<Args>(args)...));
    }

    /**
     * @brief Executes a formatted shell command without capturing output.
     *
     * Same as the overload above, but does not store the output lines. Output
     * is only streamed to logs.
     *
     * The following bash options are used:
     * -l: load /etc/profile.d/ files
     * -x: enable bash debug
     * -e: stop in the first error (use command || : to mitigate)
     * -o pipefail: return the rightmost non-zero exit code in pipelines
     *
     * Behavior depends on runtime options:
     * - If `--dry-run` is enabled, the command is not executed, only logged.
     *   are logged line-by-line for troubleshooting.
     *
     * @tparam Args Variadic template parameter pack for formatting arguments.
     * @param format The fmtlib format string for constructing the command.
     * @param args Arguments to format into the command string.
     * @return The exit code of the executed command, or 0 in dry-run mode.
     */
    template <typename... Args>
    [[nodiscard]]
    int fmt(fmt::format_string<Args...> format, Args&&... args)
    {
        return cmd(fmt::format(format, std::forward<Args>(args)...));
    }
}

/**
 * @brief Executes a shell command and throws if it fails.
 *
 * A safer wrapper around `unsafe::fmt()`. Throws std::runtime_error if the
 * command returns a non-zero exit code.
 *
 * The following bash options are used:
 * -l: load /etc/profile.d/ files
 * -x: enable bash debug
 * -e: stop in the first error (use command || : to mitigate)
 * -o pipefail: return the rightmost non-zero exit code in pipelines
 *
 * Behavior depends on runtime options:
 * - If `--dry-run` is enabled, the command is not executed, only logged.
 *
 * @tparam Args Variadic template parameter pack for formatting arguments.
 * @param format The fmtlib format string for constructing the command.
 * @param args Arguments to format into the command string.
 * @throws std::runtime_error if the command exits with a non-zero code.
 */
template <typename... Args>
void fmt(fmt::format_string<Args...> format, Args&&... args)
{
    const std::string command
        = fmt::format(format, std::forward<Args>(args)...);
    const auto exitCode = unsafe::cmd(command);
    if (exitCode != 0) {
        throw std::runtime_error(fmt::format(
            "Command {} failed with exit code {}", command, exitCode));
    }
}

/**
 * @brief Executes a raw shell command string.
 *
 * Runs the provided command string directly without formatting. Throws if
 * execution fails.
 *
 * The following bash options are used:
 * -l: load /etc/profile.d/ files
 * -x: enable bash debug
 * -e: stop in the first error (use command || : to mitigate)
 * -o pipefail: return the rightmost non-zero exit code in pipelines
 *
 * Behavior depends on runtime options:
 * - If `--dry-run` is enabled, the command is not executed, only logged.
 *
 * @param cmd The raw shell command string.
 */
void cmd(std::string_view cmd);

/**
 * @brief Executes a shell command and returns its combined output.
 *
 * Runs a command and returns its stdout as a single string, with lines
 * joined by newlines. Throws if the command fails.
 *
 * Behavior depends on runtime options:
 * - If `--dry-run` is enabled, the command is not executed, only logged.
 *
 * The following bash options are used:
 * -l: load /etc/profile.d/ files
 * -x: enable bash debug
 * -e: stop in the first error (use command || : to mitigate)
 * -o pipefail: return the rightmost non-zero exit code in pipelines
 *
 * @tparam Args Variadic template parameter pack for formatting arguments.
 * @param format The fmtlib format string for constructing the command.
 * @param args Arguments to format into the command string.
 * @return A string containing the full command output.
 * @throws std::runtime_error if the command exits with a non-zero code.
 */
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
