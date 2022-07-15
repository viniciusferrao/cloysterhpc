/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>

#include "services/log.h"
#include "const.h"
#include "cluster.h"
#include "view/newt.h"
#include "services/shell.h"
#include "presenter/PresenterInstall.h"
#include "verification.h"

#ifdef _CLOYSTER_I18N
#include "include/i18n-cpp.hpp"
#endif

// Globals definitions
bool cloyster::dryRun = false;

int main(int argc, char** argv) {
    // TODO: Parse command line options for log

    cloyster::checkEffectiveUserId();

    Log::init();
    LOG_INFO("{} Started", productName);

    auto model = std::make_unique<Cluster>();
    auto view = std::make_unique<Newt>();
    auto presenter = std::make_unique<PresenterInstall>(model, view);

#ifndef NDEBUG
    //model->fillTestData();
    model->printData();
#endif

    LOG_TRACE("Starting execution engine");
    std::unique_ptr<Execution> executionEngine = std::make_unique<Shell>(model);
    executionEngine->install();

    LOG_INFO("{} is ending", productName);
    Log::shutdown();

    return EXIT_SUCCESS;
}
