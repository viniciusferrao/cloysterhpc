/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cctype>
#include <cstdlib>


#include <CLI/CLI.hpp>
#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/hardware.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/presenter/PresenterInstall.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/verification.h>
#include <cloysterhpc/view/newt.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/dbus_client.h>
#include <internal_use_only/config.hpp>
#include <regex>

#ifdef _CLOYSTER_I18N
#include "include/i18n-cpp.hpp"
#endif



namespace {


void initializeSingletons(auto&& cluster)
{
    using cloyster::models::Cluster;
    cloyster::Singleton<Cluster>::init(
        std::forward<decltype(cluster)>(cluster));

    cloyster::Singleton<cloyster::services::IRunner>::init([](){
        using cloyster::services::IRunner;
        using cloyster::services::DryRunner;
        using cloyster::services::Runner;

        if (cloyster::dryRun) {
            return cloyster::makeUniqueDerived<IRunner, DryRunner>();
        }

        return cloyster::makeUniqueDerived<IRunner, Runner>();
    });

    using cloyster::services::repos::RepoManager;
    cloyster::Singleton<RepoManager>::init([]() {
        auto clusterPtr = cloyster::Singleton<Cluster>::get();
        const auto& osinfo = clusterPtr->getHeadnode().getOS();
        auto repoManager = std::make_unique<RepoManager>(osinfo);
        repoManager->initializeDefaultRepositories();
        return repoManager;
    });

    cloyster::Singleton<MessageBus>::init([]() {
        return cloyster::makeUniqueDerived<MessageBus, DBusClient>(
            "org.freedesktop.systemd1", "/org/freedesktop/systemd1");
    });

    cloyster::Singleton<cloyster::services::IOSService>::init([]() {
        const auto& osinfo = cloyster::Singleton<Cluster>::get()->getHeadnode().getOS();
        return cloyster::services::IOSService::factory(osinfo);
    });
}

// Run test commands and exit. This is to make easier to test
// code during development and troubleshooting.  Use a combination of
// --force and --skip to control what code to run.
int runTestCommand(const std::string& testCommand, const std::vector<std::string>& testCommandArgs)
{
#ifndef NDEBUG
    LOG_INFO("Running test command {} {} ", testCommand, fmt::join(testCommandArgs, ","));
    auto cluster = cloyster::Singleton<cloyster::models::Cluster>::get();
    auto runner = cloyster::Singleton<cloyster::IRunner>::get();
    if (testCommand == "execute-command") {
        runner->checkCommand(testCommandArgs[0]);
    } else if (testCommand == "create-http-repo") {
        assert(testCommandArgs.size() > 0);
        cloyster::createHTTPRepo(testCommandArgs[0]);
    } else if (testCommand == "parse-key-file") {
        assert(testCommandArgs.size() > 0);
        LOG_INFO("Loading file {}", testCommandArgs[0]);
        auto file = cloyster::services::files::KeyFile(testCommandArgs[0]);
        LOG_INFO("Groups: {}", fmt::join(file.getGroups(), ","));
        LOG_INFO("Contents: {}", file.toData());
    } else if (testCommand == "install-mellanox-ofed") {
        OFED(OFED::Kind::Mellanox, "latest").install();
    } else if (testCommand == "image-install-mellanox-ofed") {
        auto provisioner = std::make_unique<cloyster::services::XCAT>();
        provisioner->configureInfiniband();
    } else if (testCommand == "dump-headnode-os") {
        LOG_INFO("OS: {}", cluster->getHeadnode().getOS());
    } else if (testCommand == "dump-xcat-osimage") {
        auto provisioner = std::make_unique<cloyster::services::XCAT>();
        LOG_INFO("xCAT osimage: {}", provisioner->getImage());
    } else {
        LOG_ERROR("Invalid test command {}", testCommand);
        return EXIT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}
}; // anonymous namespace

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

    std::string dumped_answerfile;
    app.add_option("--dump-answerfile", dumped_answerfile,
        "If you pass this parameter, the software will create an answefile "
        "based on your input, and save it in the specified path");

    bool showHardwareInfo = false;
    app.add_flag("-i, --hardwareinfo", showHardwareInfo,
        "Show a detailed hardware and system overview");

    app.add_option("--customrepo", cloyster::customRepofilePath,
        "Full path to a custom repofile");

    bool unattended = false;
    app.add_flag(
        "-u, --unattended", unattended, "Perform an unattended installation");

    app.add_option("--skip", cloyster::skipSteps, "Skip a specific step during the installation");
    app.add_option("--force", cloyster::forceSteps, "Force a specific step during the installation");

#ifndef NDEBUG
    std::string testCommand{};
    app.add_option("--test", testCommand,
        "Run a command for testing purposes");

    std::vector<std::string> testCommandArgs{};
    app.add_option("--test-args", testCommandArgs,
        "Run a command for testing purposes with arguments, can be specified multiple times");
#endif

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

        // --test implies --unattended
        if (!testCommand.empty()) {
            unattended = true;
        }

        if (cloyster::dryRun) {
            LOG_INFO("Dry run enabled.");
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

        auto model = std::make_unique<cloyster::models::Cluster>();
        if (!cloyster::answerfile.empty()) {
            LOG_TRACE("Answerfile: {}", cloyster::answerfile)
            model->fillData(cloyster::answerfile);
        } else {
            cloyster::enableTUI = true;
        }

#ifndef NDEBUG
        // model->fillTestData();
        model->printData();
#endif

        if (cloyster::enableTUI && testCommand.empty()) {
            // Entrypoint; if the view is constructed it will start the TUI.
            auto view = std::make_unique<Newt>();
            auto presenter
                = std::make_unique<cloyster::presenter::PresenterInstall>(
                    model, view);
        }

        LOG_TRACE("Starting execution engine");

        if (!dumped_answerfile.empty()) {
            model->dumpData(dumped_answerfile);
        }

        initializeSingletons(std::move(model));

        std::unique_ptr<Execution> executionEngine
            = std::make_unique<cloyster::services::Shell>();

#ifndef NDEBUG
        if (!testCommand.empty()) {
            return runTestCommand(testCommand, testCommandArgs);
        }
#endif
        executionEngine->install();

    } catch (const std::exception& e) {
        LOG_ERROR("ERROR: {}", e.what());
        return EXIT_FAILURE;
    }

    LOG_INFO("{} has successfully ended", productName)
    Log::shutdown();

    return EXIT_SUCCESS;
}
