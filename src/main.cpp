/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cctype>
#include <cstdlib>

#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/const.h>
#include <cloysterhpc/dbus_client.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/presenter/PresenterInstall.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/init.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/shell.h>
#include <cloysterhpc/services/xcat.h>
#include <cloysterhpc/verification.h>
#include <cloysterhpc/view/newt.h>

#include <internal_use_only/config.hpp>

#ifdef _CLOYSTER_I18N
#include "include/i18n-cpp.hpp"
#endif

namespace {


using namespace cloyster;
using namespace cloyster::services;

// Run test commands and exit. This is to make easier to test
// code during development and troubleshooting.  Use a combination of
// --force and --skip to control what code to run.
int runTestCommand(const std::string& testCommand,
    const std::vector<std::string>& testCommandArgs)
{
#ifndef NDEBUG
    LOG_INFO("Running test command {} {} ", testCommand,
        fmt::join(testCommandArgs, ","));
    auto cluster = cloyster::Singleton<cloyster::models::Cluster>::get();
    auto runner = cloyster::Singleton<cloyster::functions::IRunner>::get();
    auto repoManager = cloyster::Singleton<repos::RepoManager>::get();
    if (testCommand == "execute-command") {
        runner->checkCommand(testCommandArgs[0]);
    } else if (testCommand == "initialize-repos") {
        repoManager->initializeDefaultRepositories();
        runner->checkCommand(R"(bash -c "dnf config-manager --set-enabled '*' && dnf makecache -y" )");
    } else if (testCommand == "create-http-repo") {
        assert(testCommandArgs.size() > 0);
        cloyster::functions::createHTTPRepo(testCommandArgs[0]);
    } else if (testCommand == "parse-key-file") {
        assert(testCommandArgs.size() > 0);
        LOG_INFO("Loading file {}", testCommandArgs[0]);
        auto file = cloyster::services::files::KeyFile(testCommandArgs[0]);
        LOG_INFO("Groups: {}", fmt::join(file.getGroups(), ",")); LOG_INFO("Contents: {}", file.toData()); } else if (testCommand == "install-mellanox-ofed") { OFED(OFED::Kind::Mellanox, "latest").install(); } else if (testCommand == "image-install-mellanox-ofed") {
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
    initializeSingletonsOptions(options::factory(argc, argv));

    auto opts = Singleton<Options>::get();
    if (opts->parsingError) {
        fmt::print("Parsing error: {}", opts->error);
        return EXIT_FAILURE;
    }

    if (opts->showVersion) {
        fmt::print("{}: Version {}\n", productName, productVersion);
        return EXIT_SUCCESS;
    }

    if (opts->helpAndExit) {
        fmt::print("Help:\n{}", opts->helpText);
        return EXIT_SUCCESS;
    }
    Log::init(opts->logLevelInput);

#ifndef NDEBUG
    LOG_DEBUG("Log level set to: {}\n", opts->logLevelInput)
#endif
    LOG_INFO("{} Started", productName)

    if (opts->testCommand.empty()) { 
        // skip during tests, we do not want to run tests as root
        cloyster::checkEffectiveUserId();
    }

    // --test implies --unattended
    if (!opts->testCommand.empty()) {
        opts->unattended = true;
    }

    if (opts->dryRun) {
        LOG_INFO("Dry run enabled.");
    } else {
        while (!opts->unattended) {
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
    if (opts->enableCLI) {
        LOG_ERROR("CLI feature not implemented.\n");
        return EXIT_FAILURE;
    }

    LOG_INFO("Initializing the model");
    auto model = std::make_unique<cloyster::models::Cluster>();
    LOG_INFO("Model initialized");
    if (!opts->answerfile.empty()) {
        LOG_INFO("Loading the answerfile: {}", opts->answerfile)
        model->fillData(opts->answerfile);
    } 

    opts->enableTUI = opts->answerfile.empty() && opts->testCommand.empty();

#ifndef NDEBUG
    // model->fillTestData();
    model->printData();
#endif

    if (opts->enableTUI) {
        // Entrypoint; if the view is constructed it will start the TUI.
        auto view = std::make_unique<Newt>();
        auto presenter
            = std::make_unique<cloyster::presenter::PresenterInstall>(
                model, view);
    }


    if (!opts->dumpAnswerfile.empty()) {
        model->dumpData(opts->dumpAnswerfile);
    }

    initializeSingletonsModel(std::move(model));

#ifndef NDEBUG
    if (!opts->testCommand.empty()) {
        return runTestCommand(opts->testCommand, opts->testCommandArgs);
    }
#endif
    LOG_TRACE("Starting execution engine");
    std::unique_ptr<Execution> executionEngine
        = std::make_unique<cloyster::services::Shell>();

    executionEngine->install();

    LOG_INFO("{} has successfully ended", productName)
    Log::shutdown();

    return EXIT_SUCCESS;
}
