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
bool cloyster::dryRun = true;

/**
 * @brief The entrypoint.
 */
int main(int argc, const char** argv)
{
    CLI::App app{ productName };


    bool showVersion = false;
    app.add_flag("-v,--version", showVersion, "Show version information");

    bool runAsRoot = false;
    app.add_flag("-r, --root", runAsRoot, "Run with root permissions");

    bool dryRun = true;
    app.add_flag("-d, --dryrun", dryRun, "Perform a dry run installation");

    bool enableTUI = false;
    app.add_flag("-t, --tui", enableTUI, "Enable TUI");

    std::optional<std::string> logLevelInput;
    app.add_option("-l, --log-level", logLevelInput, (fmt::format("Available log levels: {}", fmt::join(magic_enum::enum_names<Log::Level>(), ", "))) );

    CLI11_PARSE(app, argc, argv);

    Log::Level logLevel = Log::Level::Info;
    if (logLevelInput) {
        if (std::regex_match(logLevelInput.value(), std::regex("^[0-9]+$")))
            logLevel = magic_enum::enum_cast<Log::Level>(stoi(logLevelInput.value())).value();
        else
            logLevel = magic_enum::enum_cast<Log::Level>(logLevelInput.value()).value();
    }

    try {
        Log::init(logLevel);
        LOG_INFO("{} Started", productName);
    } catch (const std::exception& ex) {
        fmt::print("Invalid log level: {}\n{}", logLevelInput.value(), ex.what());
        return EXIT_FAILURE;
    }

#ifndef NDEBUG
    fmt::print("Log level set to: {}", magic_enum::enum_name<Log::Level>(logLevel));
#endif

    if (showVersion) {
        fmt::print("{}: Version {}\n", productName, productVersion);
        return EXIT_SUCCESS;
    }

    if(runAsRoot)
        cloyster::checkEffectiveUserId();

    if (dryRun) {
        fmt::print("Dry run enabled.\n");
        cloyster::dryRun = dryRun;
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
