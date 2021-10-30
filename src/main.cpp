/* C++ includes */
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
#include "presenter.h"

int main(int argc, char** argv) {
    int rc; /* return code */
    auto view = std::make_unique<viewTerminalUI>();
    //auto viewPtr = view.get();
    auto model = std::make_unique<Cluster>();
    auto modelPtr = model.get();

    auto presenter = std::make_unique<Presenter>(std::move(view),
                                                 std::move(model));

    //welcomeMessage();
    //timezoneSelection({"Teste1", "Teste2"});
    //m_subscriber->notifyEvent();

#if 0
    /* TerminalUI entrypoint */
    Cluster cluster;
    Presenter presenter(cluster);
#endif

#ifdef _DEBUG_
    modelPtr->printData();
#endif

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
    cluster.timezone = "America/Sao_Paulo";
    cluster.locale = "en_US.utf8";
    cluster.fqdn = "headnode.cloyster.invalid";
    cluster.firewall = false;
    cluster.selinux = false;

#ifndef _DUMMY_
    if (headnode.os.family == OS::Family::Darwin) {
        std::cerr << "ERROR: Cowardly refusing to run commands on macOS!" <<
                                                                    std::endl;
        exit(-1);
    }
#endif
        cluster.install();
#endif

    return EXIT_SUCCESS;
}
