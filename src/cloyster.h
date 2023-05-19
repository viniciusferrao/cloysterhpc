/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CLOYSTER_H_
#define CLOYSTERHPC_CLOYSTER_H_

#include "services/log.h"
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <string>

namespace cloyster {
bool showVersion = false;
bool runAsRoot = false;
bool dryRun = false;
bool enableTUI = false;
bool enableCLI = false;
bool runAsDaemon = false;
std::string logLevelInput
    = fmt::format("{}", magic_enum::enum_name(Log::Level::Info));
std::string answerfile;
}

#endif // CLOYSTERHPC_CLOYSTER_H_
