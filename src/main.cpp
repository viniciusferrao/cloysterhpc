/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>

#include "cloyster.h"
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

/**
 * @brief The entrypoint.
 */
int main(int argc, const char** argv)
{
    CLI::App app { productName };

    app.add_flag(
        "-v, --version", cloyster::showVersion, "Show version information");

    app.add_flag(
        "-r, --root", cloyster::runAsRoot, "Run with root permissions");

    app.add_flag(
        "-d, --dry", cloyster::dryRun, "Perform a dry run installation");

    app.add_flag("-t, --tui", cloyster::enableTUI, "Enable TUI");

    app.add_flag("-c, --cli", cloyster::enableCLI, "Enable CLI");

    app.add_flag("-D, --daemon", cloyster::runAsDaemon, "Run as a daemon");

    cloyster::logLevelInput
        = fmt::format("{}", magic_enum::enum_name(Log::Level::Info));
    constexpr std::size_t logLevels { magic_enum::enum_count<Log::Level>() };

    const std::vector<std::string> logLevelVector = []() {
        constexpr const auto& logLevelNames {
            magic_enum::enum_names<Log::Level>()
        };
        return std::vector<std::string> { logLevelNames.begin(),
            logLevelNames.end() };
    }();

    app.add_option("-l, --log-level", cloyster::logLevelInput,
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

    app.add_option("-a, --answerfile", cloyster::answerfile, "Full path to a answerfile");

    CLI11_PARSE(app, argc, argv)

    Log::init([]() {
        if (std::regex_match(cloyster::logLevelInput, std::regex("^[0-9]+$"))) {
            return magic_enum::enum_cast<Log::Level>(
                stoi(cloyster::logLevelInput))
                .value();
        } else {
            return magic_enum::enum_cast<Log::Level>(
                cloyster::logLevelInput, magic_enum::case_insensitive)
                .value();
        }
    }());

#ifndef NDEBUG
    LOG_DEBUG("Log level set to: {}\n", cloyster::logLevelInput);
#endif

    LOG_INFO("{} Started", productName);

    if (cloyster::showVersion) {
        fmt::print("{}: Version {}\n", productName, productVersion);
        return EXIT_SUCCESS;
    }

    if (cloyster::runAsRoot) {
        cloyster::checkEffectiveUserId();
    }

    if (cloyster::dryRun) {
        fmt::print("Dry run enabled.\n");
    }

    //@TODO implement CLI feature
    if (cloyster::enableCLI) {
        fmt::print("CLI feature not implemented.\n");
        return EXIT_FAILURE;
    }

    //@TODO implement run as daemon feature
    if (cloyster::runAsDaemon) {
        fmt::print("Daemon feature not implemented.\n");
        return EXIT_FAILURE;
    }

    auto model = std::make_unique<Cluster>();

    if (!cloyster::answerfile.empty()) {
        LOG_TRACE("Answerfile: {}", cloyster::answerfile);
        model->fillData(cloyster::answerfile);
    }

    if (cloyster::enableTUI) {
        // Entrypoint; if the view is constructed it will start the TUI.
        auto view = std::make_unique<Newt>();
        auto presenter = std::make_unique<PresenterInstall>(model, view);
    }


#ifndef NDEBUG
    model->fillTestData();
    model->printData();
#endif

    LOG_TRACE("Starting execution engine");
    std::unique_ptr<Execution> executionEngine = std::make_unique<Shell>(model);
    executionEngine->install();

    LOG_INFO("{} is ending", productName);
    Log::shutdown();

    return EXIT_SUCCESS;
}
