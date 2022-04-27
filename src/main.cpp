#include <iostream>
#include <cstdlib>

#define ENABLE_GETOPT_H
#ifdef ENABLE_GETOPT_H
#include <getopt.h>
#endif

#include "services/log.h"
#include "const.h"
#include "functions.h"
#include "headnode.h"
#include "cluster.h"
#include "view/newt.h"
#include "services/shell.h"
#include "presenter/PresenterInstall.h"

#ifdef _CLOYSTER_I18N
#include "include/i18n-cpp.hpp"
#endif

// Globals definition
bool cloyster::dryRun = false;

int main(int argc, char** argv) {
    // TODO: Parse command line options for log levels
    Log::init();
    LOG_INFO("{} Started", productName);

    auto model = std::make_unique<Cluster>();

#ifdef _DEBUG_
    //model->fillTestData();
#endif

    auto view = std::make_unique<Newt>();
    auto presenter = std::make_unique<PresenterInstall>(model, view);

#ifdef _DEBUG_
    model->printData();
#endif

    LOG_TRACE("Starting execution engine");
    std::unique_ptr<Execution> executionEngine = std::make_unique<Shell>(model);
    executionEngine->install();

#if 0 /* Porting TerminalUI */
    /* .conf file manipulation */
    std::string homeDirectory = getEnvironmentVariable("HOME");
    std::string configFile = homeDirectory + "/.cloyster.conf";
    readConfig(configFile); /* crashes if file does not exist */
    writeConfig(configFile);

    Headnode headnode;
    if ((rc = headnode.setOS())) {
        std::cerr << "Failed to setOS: return code " << rc << std::endl;
        exit(-1);
    }

#ifdef _DEBUG_
    headnode.printOS();
#endif

#ifndef _DUMMY_
    if (headnode.m_os.family == OS::Family::Darwin) {
        std::cerr << "ERROR: Cowardly refusing to run commands on macOS!" <<
                                                                    std::endl;
        exit(-1);
    }
#endif
        cluster.install();
#endif

    LOG_INFO("{} is ending", productName);
    Log::shutdown();
    return EXIT_SUCCESS;
}
