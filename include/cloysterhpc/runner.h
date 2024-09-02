/*
 * Copyright 2024 Arthur Mendes <arthur@versatushpc.com.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

/**
 * Works as an abstraction for command execution.
 */
class BaseRunner {
public:
    virtual int executeCommand(const std::string&) = 0;

    virtual int downloadFile(const std::string& url, const std::string& file);

    virtual ~BaseRunner() = default;
};

class Runner : public BaseRunner {
public:
    int executeCommand(const std::string&) override;

    virtual ~Runner() = default;
};

class DryRunner : public BaseRunner {
public:
    int executeCommand(const std::string&) override;

    virtual ~DryRunner() = default;
};

class MockRunner : public BaseRunner {
public:
    int executeCommand(const std::string&) override;

    virtual ~MockRunner() = default;

    [[nodiscard]] const std::vector<std::string>& listCommands() const;

private:
    std::vector<std::string> m_cmds;
};
