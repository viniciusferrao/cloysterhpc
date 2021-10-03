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
#include "server.hpp"
#include "cluster.hpp"
//#include "tui.hpp"

const char* const ibStacks[] = {
    "None",
    "Inbox",
    "Mellanox",
    NULL
};

const char* const queueSystems[] = {
    "None",
    "SLURM",
    "PBS Professional",
    NULL
};

const char* const pbsDefaultPlace[] = {
    "Shared",
    "Scatter",
    NULL
};

const char* const postfixProfiles[] = {
    "Local",
    "Relay",
    "SASL",
    NULL
};

/* getopt code:
 * return optind to manipulate argc and argv outside funcion
 * if GETOPT_H is not available will always return 0
 * 
 * Important things to note:
 * If an option was successfully found, then getopt() returns the option 
 * character. If all command-line options have been parsed, then getopt()
 * returns -1. If getopt() encounters an option character that was not in 
 * optstring, then '?' is returned. If getopt() encounters an option with a
 * missing argument, then the return value depends on the first character in
 * optstring: if it is ':', then ':' is returned; otherwise '?' is returned.
 * https://www.man7.org/linux/man-pages/man3/getopt.3.html
 */
int parseArguments(int argc, char **argv) {
    int ch;
    const char *shortOptions = ":a:hv";

    while (true) {
#ifdef ENABLE_GETOPT_H
        int optionIndex = 0;
        static struct option const longOptions[] = {
            {"help", no_argument, NULL, 'h'},
            {"version", no_argument, NULL, 'v'},
            {NULL, 0, NULL, 0}
        };

        ch = getopt_long(argc, argv, shortOptions, longOptions, &optionIndex);
#else
        ch = getopt(argc, argv, shortOptions);
#endif
        if (ch == -1)
            break;

        switch (ch) {
            case 'a':
                printf("Argumento passado: %s\n", optarg);
                break;
            case 'h':
                printf("HELP PLACEHOLDER\n");
                break;
            case 'v':
                printf("Development Version\n");
                break;
            case ':':
                printf("Missing argument for option -%c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case 0:
            case '?':
                printf("Unknown option -%c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            default:
                printf("Usage: %s [OPTION]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* I have no idea why this is needed */
    /*argc -= optind;
    argv += optind;
    */

#ifdef ENABLE_GETOPT_H
    return optind;
#else
    return 0;
#endif
}

int main(int argc, char **argv) {
    int rc; /* return code */

#if 0 /* Not ported yet to C++; broken */
    if (argc == 1) {
        /* Start up the graphical terminal interface */
        tuiStartup();
        exit(0);

#ifdef _DEBUG_
        tuiDebugInfo();
#endif

        tuiBeginInstall(&cluster);
        tuiExit();
    } else {
#endif
        optind = parseArguments(argc, argv);

        argc -= optind;
        argv += optind;
#if 0
    }
#endif

    Server headnode;
    if ((rc = headnode.setOS())) {
        std::cout << "Failed to setOS: return code " << rc << std::endl;
    }

    if ((rc = headnode.checkSupportedOS())) {
        std::cout << "Unsupported OS: return code " << rc << std::endl;
    }

    /* At this point we can start the installation */

#if 0
    /* execvp test */
    char *argumentList[] = {"ls", "-l", NULL};
    int statusCode = execvp(argumentList[0], argumentList);
#endif

#if 0
    /* popen test */
    FILE* file = popen("ls -l", "r"); // You should add error checking here.
    char *out = NULL;
    size_t outlen = 0;
    while (getline(&out, &outlen, file) >= 0)
    {
        printf("%s", out);
        printf("oi\n");
    }
    pclose(file);
    free(out);
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
