/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/dnf.h>
#include <fmt/format.h>

bool dnf::install(std::string_view package)
{
    return (
        cloyster::runCommand(fmt::format("dnf -y install {}", package)) != 0);
}

bool dnf::remove(std::string_view package)
{
    return (
        cloyster::runCommand(fmt::format("dnf -y remove {}", package)) != 0);
}

bool dnf::update(std::string_view package)
{
    return (
        cloyster::runCommand(fmt::format("dnf -y update {}", package)) != 0);
}

bool dnf::update() { return (cloyster::runCommand("dnf -y update") != 0); }

void dnf::check() { cloyster::runCommand("dnf check"); }

void dnf::clean() { cloyster::runCommand("dnf clean all"); }

std::list<std::string> dnf::repolist()
{
    std::list<std::string> output;
    cloyster::runCommand("dnf repolist", output, true);
    return output;
}
