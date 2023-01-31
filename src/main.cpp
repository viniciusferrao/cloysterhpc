/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>

#include "cluster.h"
#include "const.h"
#include "presenter/PresenterInstall.h"
#include "services/log.h"
#include "services/shell.h"
#include "verification.h"
#include "view/newt.h"
#include <CLI/CLI.hpp>

#ifdef _CLOYSTER_I18N
#include "include/i18n-cpp.hpp"
#endif

// Globals definitions
bool cloyster::dryRun = false;

/**
 * @brief The entrypoint.
 */
int main(int argc, const char** argv)
{
    CLI::App app { productName };

    bool showVersion = false;
    app.add_flag("-v, --version", showVersion, "Show version information");

    bool runAsRoot = false;
    app.add_flag("-r, --root", runAsRoot, "Run with root permissions");

    bool dryRun = false;
    app.add_flag("-d, --dry", dryRun, "Perform a dry run installation");

    bool enableTUI = false;
    app.add_flag("-t, --tui", enableTUI, "Enable TUI");

    bool enableCLI = false;
    app.add_flag("-c, --cli", enableCLI, "Enable CLI");

    bool runAsDaemon = false;
    app.add_flag("-D, --daemon", runAsDaemon, "Run as a daemon");

    std::string logLevelInput = fmt::format("{}", magic_enum::enum_name(Log::Level::Info));
    constexpr std::size_t logLevels { magic_enum::enum_count<Log::Level>() };

    const std::vector<std::string> logLevelVector = []() {
        constexpr auto& logLevelNames { magic_enum::enum_names<Log::Level>() };
        return std::vector<std::string> { logLevelNames.begin(),
            logLevelNames.end() };
    }();

    app.add_option("-l, --log-level", logLevelInput,
           [&logLevelVector]() {
               std::string result { "Available log levels:" };

               for (std::size_t i = 0; const auto& logLevel : logLevelVector) {
                   result += fmt::format(" {} ({}),", logLevel, i++);
               }

               result.pop_back();
               return result;
           }())
        ->check(CLI::IsMember(logLevelVector, CLI::ignore_case)
            | CLI::Range(logLevels - 1))
        // This is a hack to solve the autogen string:
        // -l,--log-level TEXT:({Trace,Debug,Info,Warn,Error,Critical,Off})
        // OR (INT in [0 - 6])
        ->option_text(" ");

    CLI11_PARSE(app, argc, argv)

    Log::init([&logLevelInput]() {
        if (std::regex_match(logLevelInput, std::regex("^[0-9]+$")))
            return magic_enum::enum_cast<Log::Level>(stoi(logLevelInput))
                .value();
        else
            return magic_enum::enum_cast<Log::Level>(
                logLevelInput, magic_enum::case_insensitive)
                .value();
    }());

#ifndef NDEBUG
    fmt::print("Log level set to: {}\n", logLevelInput);
#endif


    LOG_INFO("{} Started", productName);

    if (showVersion) {
        fmt::print("{}: Version {}\n", productName, productVersion);
        return EXIT_SUCCESS;
    }

    if (runAsRoot)
        cloyster::checkEffectiveUserId();

    if (dryRun) {
        fmt::print("Dry run enabled.\n");
        cloyster::dryRun = dryRun;
    }

    //@TODO implement CLI feature
    if (enableCLI) {
        fmt::print("CLI feature not implemented.\n");
        return EXIT_FAILURE;
    }

    //@TODO implement run as daemon feature
    if (runAsDaemon) {
        fmt::print("Daemon feature not implemented.\n");
        return EXIT_FAILURE;
    }

    auto model = std::make_unique<Cluster>();
    if (enableTUI) {
        // Entrypoint; if the view is constructed it will start the TUI.
        auto view = std::make_unique<Newt>();
        auto presenter = std::make_unique<PresenterInstall>(model, view);
    }




#ifndef NDEBUG
    //    model->fillTestData();
    model->printData();
#endif

    LOG_TRACE("Starting execution engine");
    std::unique_ptr<Execution> executionEngine = std::make_unique<Shell>(model);
    executionEngine->install();

    LOG_INFO("{} is ending", productName);
    Log::shutdown();

    return EXIT_SUCCESS;
}
