/*
 * Copyright 2024 Arthur Mendes <arthur@versatushpc.com.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_RUNNER_H_
#define CLOYSTERHPC_RUNNER_H_

#include <boost/process.hpp>

#include <string>
#include <vector>

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
    virtual CommandProxy executeCommandIter(
        const std::string&, Stream out = Stream::Stdout)
        = 0;
    virtual void checkCommand(const std::string&) = 0;
    virtual std::vector<std::string> checkOutput(const std::string&) = 0;
    virtual int downloadFile(const std::string& url, const std::string& file)
        = 0;
};

class Runner final : public IRunner {
public:
    int executeCommand(const std::string& cmd) override;
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;
};

class DryRunner final : public IRunner {
public:
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    int executeCommand(const std::string& cmd) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;
};

class MockRunner final : public IRunner {
public:
    CommandProxy executeCommandIter(
        const std::string& cmd, Stream out = Stream::Stdout) override;
    int executeCommand(const std::string& cmd) override;
    void checkCommand(const std::string& cmd) override;
    std::vector<std::string> checkOutput(const std::string& cmd) override;
    int downloadFile(const std::string& url, const std::string& file) override;

    [[nodiscard]] const std::vector<std::string>& listCommands() const;

private:
    std::vector<std::string> m_cmds;
};

} // namespace cloyster::services

#endif // CLOYSTERHPC_RUNNER_H_
