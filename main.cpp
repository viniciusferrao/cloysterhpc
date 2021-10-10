/* C++ includes */
#include <iostream>
#include <cstdlib>

#define ENABLE_GETOPT_H
#ifdef ENABLE_GETOPT_H
#include <getopt.h>
#endif

#include "error.hpp"
#include "const.hpp"
#include "types.hpp"
#include "messages.hpp"
#include "functions.hpp"
#include "headnode.hpp"
#include "cluster.hpp"
#include "terminalui.hpp"

int main(int argc, char **argv) {
    int rc; /* return code */

    /* TerminalUI entrypoint */
    TerminalUI terminalui;

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

#if 0
    printf("Cluster attributes defined:\n");
    printf("Timezone: %s\n", cluster.timezone);
    printf("Locale: %s\n", cluster.locale);
    printf("Hostname: %s\n", cluster.hostname);
    printf("Domainname: %s\n", cluster.domainname);
    printf("FQDN: %s\n", cluster.fqdn);
    printf("interfaceExternal: %s\n", cluster.interfaceExternal);
    printf("interfaceInternal: %s\n", cluster.interfaceInternal);
    printf("interfaceInternalNetwork: %s\n", cluster.interfaceInternalNetwork);
    printf("interfaceInternalIP: %s\n", cluster.interfaceInternalIP);
    printf("xCATDynamicRangeStart: %s\n", cluster.xCATDynamicRangeStart);
    printf("xCATDynamicRangeEnd: %s\n", cluster.xCATDynamicRangeEnd);

    printf("Directory Admin Password: %s\n", cluster.directoryAdminPassword);
    printf("Directory Manager Password: %s\n", cluster.directoryManagerPassword);
    printf("Directory Disable DNSSEC: %s\n", cluster.directoryDisableDNSSEC ? "true" : "false");

    printf("nodePrefix: %s\n", cluster.nodePrefix);
    printf("nodePadding: %s\n", cluster.nodePadding);
    printf("nodeStartIP: %s\n", cluster.nodeStartIP);
    printf("nodeRootPassword: %s\n", cluster.nodeRootPassword);
    printf("nodeISOPath: %s\n", cluster.nodeISOPath);

    printf("ibStack: %s\n", cluster.ibStack);

    printf("queueSystem: %s\n", cluster.queueSystem.name);
    if (strstr(cluster.queueSystem.name, "SLURM"))
        printf("slurm.partition: %s\n", cluster.queueSystem.slurm.partition);
    if (strstr(cluster.queueSystem.name, "PBS"))
        printf("pbs.defaultPlace: %s\n", cluster.queueSystem.pbs.defaultPlace);

    printf("Enable Postfix: %s\n", cluster.postfix.enable ? "true" : "false");
    if (cluster.postfix.enable) {
        printf("\t-> Profile: %s\n", postfixProfiles[cluster.postfix.profileId]);
        switch (cluster.postfix.profileId) {
            case 0:
                /* Local */
                break;
            case 1:
                /* Relay */
                printf("\t\t-> Hostname: %s\n", cluster.postfix.relay.hostname);
                printf("\t\t-> Port: %u\n", cluster.postfix.relay.port);
                break;
            case 2:
                /* SASL */
                printf("\t\t-> Hostname: %s\n", cluster.postfix.sasl.hostname);
                printf("\t\t-> Port: %u\n", cluster.postfix.sasl.port);
                printf("\t\t-> Username: %s\n", cluster.postfix.sasl.username);
                printf("\t\t-> Password: %s\n", cluster.postfix.sasl.password);
                break;
        }
    }

    printf("Update system: %s\n", cluster.updateSystem ? "true" : "false");
    printf("Remote access: %s\n", cluster.remoteAccess ? "true" : "false");
#endif

    return EXIT_SUCCESS;
}
