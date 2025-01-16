/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CLOYSTER_H_
#define CLOYSTERHPC_CLOYSTER_H_

#include <cloysterhpc/services/log.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <string>

namespace cloyster {
extern bool showVersion;
extern bool runAsRoot;
extern bool dryRun;
extern bool enableTUI;
extern bool enableCLI;
extern bool runAsDaemon;

extern std::string logLevelInput;
extern std::string answerfile;
extern std::string customRepofilePath;
}

#endif // CLOYSTERHPC_CLOYSTER_H_
