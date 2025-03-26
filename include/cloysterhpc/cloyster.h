/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CLOYSTER_H_
#define CLOYSTERHPC_CLOYSTER_H_

#include <cloysterhpc/services/log.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <set>
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

extern std::set<std::string> skipSteps;
extern std::set<std::string> forceSteps;

bool shouldSkip(const std::string& step);
bool shouldForce(const std::string& step);
}

#endif // CLOYSTERHPC_CLOYSTER_H_
