#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include <iostream>
#include <cstdlib>

#define ENABLE_GETOPT_H
#ifdef ENABLE_GETOPT_H
#include <getopt.h>
#endif

#include "error.h"
#include "const.h"
#include "types.h"
#include "messages.h"
#include "functions.h"
#include "headnode.h"
#include "cluster.h"
#include "connection.h"
#include "terminalui.h"
#include "presenter/presenterBase.h"
#include "presenter/presenterWelcome.h"
#include "view/newtViewBase.h"
#include "view/newtViewMessage.h"
#include "services/shell.h"

int main(int argc, char** argv) {
    int rc; /* return code */

    //Cluster* model = new Cluster();
    auto model = std::make_unique<Cluster>();

#if 0
    //View* view = new NewtViewBase();
    auto view = std::make_unique<NewtViewMessage>();

    //PresenterBase* presenter = new PresenterBase(std::move(view),
    //                                             std::move(model));
    auto presenter = std::make_unique<PresenterWelcome>(std::move(view),
                                                     std::move(model));
#endif

#ifdef _DEBUG_
    model->fillTestData();
    model->printData();
#endif

    //Execution* engine = new Shell();
    std::unique_ptr<Execution> executionEngine = std::make_unique<Shell>();
    executionEngine->install(model);
    //delete engine;



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

    if ((rc = headnode.checkSupportedOS())) {
        std::cerr << "Unsupported OS: return code " << rc << std::endl;
    }

    /* Testing */
    std::string CSIIPAddress = "146.164.36.16";
    if (headnode.network.setIPAddress(CSIIPAddress) != 0)
        std::cerr << "Invalid IPv4,5 address" << std::endl;
    std::cout << headnode.network.getIPAddress() << std::endl;

    //headnode.network.setProfile(Network::Profile::Application);
    if (headnode.network.getProfile() == Network::Profile::External)
        std::cout << "External" << std::endl;

    //headnode.network.setType(Network::Type::Infiniband);
    if (headnode.network.getType() == Network::Type::Ethernet)
        std::cout << "Ethernet" << std::endl;

    /* At this point we can start the installation */
    Cluster cluster;

    /* Trash initialization */
    cluster.m_timezone = "America/Sao_Paulo";
    cluster.m_locale = "en_US.utf8";
    cluster.m_fqdn = "headnode.cloyster.invalid";
    cluster.m_firewall = false;
    cluster.m_selinux = false;

#ifndef _DUMMY_
    if (headnode.m_os.family == OS::Family::Darwin) {
        std::cerr << "ERROR: Cowardly refusing to run commands on macOS!" <<
                                                                    std::endl;
        exit(-1);
    }
#endif
        cluster.install();
#endif

    return EXIT_SUCCESS;
}
