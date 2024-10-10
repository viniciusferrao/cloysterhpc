/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cctype>
#include <cstdlib>

#include "cloysterhpc/hardware.h"
#include <CLI/CLI.hpp>
#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/cluster.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/presenter/PresenterInstall.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/verification.h>
#include <cloysterhpc/view/newt.h>
#include <internal_use_only/config.hpp>

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
        "-d, --dry", cloyster::dryRun, "Perform a dry run installation");

    app.add_flag("-t, --tui", cloyster::enableTUI, "Enable TUI");

#if 0    
    app.add_flag("-c, --cli", cloyster::enableCLI, "Enable CLI");
#endif

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

    app.add_option(
        "-a, --answerfile", cloyster::answerfile, "Full path to a answerfile");

    bool showHardwareInfo = false;
    app.add_flag("-i, --hardwareinfo", showHardwareInfo,
        "Show a detailed hardware and system overview");

    app.add_option("--customrepo", cloyster::customRepofilePath,
        "Full path to a custom repofile");

    bool unattended = false;
    app.add_flag(
        "-u, --unattended", unattended, "Perform an unattended installation");

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

    if (cloyster::showVersion) {
        fmt::print("{}: Version {}\n", productName, productVersion);
        return EXIT_SUCCESS;
    }

    try {
#ifndef NDEBUG
        LOG_DEBUG("Log level set to: {}\n", cloyster::logLevelInput)
#endif
        LOG_INFO("{} Started", productName)

        if (showHardwareInfo) {
            Hardware hardware;
            hardware.printOverview();
            return EXIT_SUCCESS;
        }

        cloyster::checkEffectiveUserId();

        if (cloyster::dryRun) {
            fmt::print("Dry run enabled.\n");
        } else {
            while (!unattended) {
                char response = 'N';
                fmt::print("{} will now modify your system, do you want to "
                           "continue? [Y/N]\n",
                    cloyster::productName);
                std::cin >> response;

                if (std::toupper(response) == 'Y') {
                    LOG_INFO("Running {}.\n", cloyster::productName)
                    break;
                } else if (std::toupper(response) == 'N') {
                    LOG_INFO("Stopping {}.\n", cloyster::productName)
                    return EXIT_SUCCESS;
                }
            }
        }

        //@TODO implement CLI feature
        if (cloyster::enableCLI) {
            fmt::print("CLI feature not implemented.\n");
            return EXIT_FAILURE;
        }

        auto model = std::make_unique<Cluster>();

        if (!cloyster::answerfile.empty()) {
            LOG_TRACE("Answerfile: {}", cloyster::answerfile)
            model->fillData(cloyster::answerfile);
        } else {
            cloyster::enableTUI = true;
        }

        if (cloyster::enableTUI) {
            // Entrypoint; if the view is constructed it will start the TUI.
            auto view = std::make_unique<Newt>();
            auto presenter = std::make_unique<PresenterInstall>(model, view);
        }

#ifndef NDEBUG
        // model->fillTestData();
        model->printData();
#endif

        LOG_TRACE("Starting execution engine")
        std::unique_ptr<Execution> executionEngine
            = std::make_unique<Shell>(model);
        executionEngine->install();

    } catch (const std::exception& e) {
        LOG_ERROR("ERROR: {}", e.what());
        return EXIT_FAILURE;
    }

    LOG_INFO("{} has successfully ended", productName)
    Log::shutdown();

    return EXIT_SUCCESS;
}
