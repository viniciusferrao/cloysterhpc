//
// Created by Lucas Gracioso
//

#include <cstdlib>

#include "presenter/PresenterInstall.h"
#include <CLI/CLI.hpp>

namespace cloyster {
    bool showVersion = false;
    bool runAsRoot = false;
    bool dryRun = false;
    bool enableTUI = false;
    bool enableCLI = false;
    bool runAsDaemon = false;
    std::string logLevelInput = fmt::format("{}", magic_enum::enum_name(Log::Level::Info));
}

#ifndef CLOYSTERHPC_CLOYSTER_H_
#define CLOYSTERHPC_CLOYSTER_H_

#endif // CLOYSTERHPC_CLOYSTER_H_
