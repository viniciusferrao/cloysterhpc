/* TUI (Terminal UI) implementation using newt.
 * This file must be ported to C++ it's still in C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <newt.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "tui.hpp"
#include "const.hpp"
#include "types.hpp"
#include "error.hpp"
#include "messages.hpp"

bool tuiHasEmptyField(const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(NULL, MSG_BUTTON_OK, message);
            return true;
        }
    }

    return false;
}

/* This function checks if a given IP address is valid or not, it should be
 * noted that format should be without the mask information, everything that
 * applies to to the inet_aton function applies here:
 * https://linux.die.net/man/3/inet_addr
 */
bool tuiIsValidIP(const char *string) {
    struct in_addr ip;

    if (!inet_aton(string, &ip))
        return false;

    return true;
}

#ifdef _DEBUG_
void tuiDebugEntries(const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 255 chars */
    char message[255] = {};

    /* The ideia on this loop is to read all the available data from *entries,
     * so we add the size of the string in order to give padding for additional
     * data. In the first iteraion strlen() should be zero, and only in the
     * following iterations data will be added to the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++)
        sprintf(message + strlen(message), "%s -> %s\n", entries[i].text, 
                *entries[i].value);
    
    newtWinMessage(NULL, MSG_BUTTON_OK, message);
}
#endif

void tuiStartup(void) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();
}

void tuiExit(void) {
    newtFinished();
}

void tuiAbortInstall(void) {
    tuiExit();
    printf(MSG_INSTALL_ABORT);
    exit(EXIT_SUCCESS);
}

/* This is the default help action function, that should be called whatever the
 * operator asks for help.
 * The nature of the function relies on const char* for safety reasons but the
 * underling function requires char *; so we basically need to duplicate the
 * string to a temporary one just to display the message.
 */
void tuiHelpMessage(const char* message) {
    char* temp = strdup(message);

    newtBell();
    newtWinMessage(MSG_TITLE_HELP, MSG_BUTTON_OK, temp);
}

/* The first implementation is just a run over of the old questions from the
 * ansible playbook, we will mimic the old behavior to enhance it afterwards.
 */
void tuiBeginInstall(CLUSTER *cluster) {
#if 0
    tuiWelcomeMessage();
    tuiTimeSettings(cluster);
    tuiLocaleSettings(cluster);
    tuiNetworkSettings(cluster);
    tuiDirectoryServicesSettings(cluster);
    tuiNodeSettings(cluster);
    tuiInfinibandSettings(cluster);
    tuiQueueSystemSettings(cluster);
    tuiPostfixSettings(cluster);
    tuiUpdateSystem(cluster);
    tuiRemoteAccessSettings(cluster);
#endif
    tuiIPv4Settings(cluster);
}

void tuiWelcomeMessage(void) {
    int returnValue;

    /* We start pushing thw welcome message */
    newtWinMessage(NULL, MSG_BUTTON_OK, MSG_WELCOME);

    /* Information about the installation scheme */
    returnValue = newtWinChoice(NULL, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_GUIDED_INSTALL);

    switch (returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            break;
        case 2:
            tuiAbortInstall();
    }
}

void tuiTimeSettings(CLUSTER *cluster) {
    int returnValue;
    int selector = 0;

    /* This is a placeholder const until we figure out how to fetch the list of
     * supported timezones from the OS instead.
     */
    const char* const timezones[] = {
        "America/Sao_Paulo",
        "UTC",
        "Gadific Mean Bolsotime",
        "Chronus",
        "Two blocks ahead",
        NULL 
    };

    question0:    
    returnValue = newtWinMenu(MSG_TITLE_TIME_SETTINGS, MSG_TIME_SETTINGS_TIMEZONE, 50,
                    5, 5, 3, (char**) timezones, &selector, MSG_BUTTON_OK, 
                    MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->timezone = strdup(timezones[selector]);
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_TIME_SETTINGS_TIMEZONE_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

void tuiLocaleSettings(CLUSTER *cluster) {
    int returnValue;
    int selector = 0;

    /* This is a placeholder const until we figure out how to fetch the list of
     * supported locales from the OS instead.
     */
    const char* const locales[] = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C",
        NULL
    };

    question0:    
    returnValue = newtWinMenu(MSG_TITLE_LOCALE_SETTINGS, MSG_LOCALE_SETTINGS_LOCALE, 50, 5, 5, 3,
                    (char**) locales, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->locale = strdup(locales[selector]);
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_LOCALE_SETTINGS_LOCALE_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}    

/* We ask the required networking info to provide the cluster
 * This function is not that good, the data modelling isn't good either. Perhaps
 * we should have a function to handle IPv4 questions and call it when needed.
 * Questions about network are much the same on the service and application
 * networks, so it's duplicated code. xCAT does not have a good fit here too, it
 * should be modular so we can remove xCAT if we need to.
 */
void tuiNetworkSettings(CLUSTER *cluster) {
    int returnValue;
    //char **netInterfaces;
    /* Implement with https://linux.die.net/man/3/getifaddrs */
    char *netInterfaces[] = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0",
        NULL
    };
    int selector = 0;

    /* Request hostname and domain name */
    char *hostIdEntries[2];
    struct newtWinEntry hostId[] = {
        { "Hostname", hostIdEntries + 0, 0 },
        { "Domain name", hostIdEntries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(hostIdEntries, 0, sizeof(hostIdEntries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_NETWORK_SETTINGS, MSG_NETWORK_SETTINGS_HOSTID, 50, 5, 5, 20,
        hostId, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(hostId))
                goto question0;      
#ifdef _DEBUG_
            tuiDebugEntries(hostId);
#endif
            cluster->hostname = strdup(*hostId[0].value);
            cluster->domainname = strdup(*hostId[1].value);
#if 1
            /* This malloc is just wrong; we ask two times the memory */
            char *fqdn = (char *) calloc(2, sizeof(cluster->hostname) + sizeof(cluster->domainname) + 1);
            sprintf(fqdn, "%s.%s", cluster->hostname, cluster->domainname);
            cluster->fqdn = strdup(fqdn);
#endif
            /* Clanup the memory */
            for (unsigned i = 0; i < sizeof (hostId) / sizeof (struct newtWinEntry) && hostId[i].value; i++)
                free(*(hostId[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NETWORK_SETTINGS_HOSTID_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }

    question1:
    returnValue = newtWinMenu(MSG_TITLE_NETWORK_SETTINGS, MSG_NETWORK_SETTINGS_EXTERNAL_IF, 50, 5, 5, 3, 
		     netInterfaces, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->interfaceExternal = netInterfaces[selector];
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);
            goto question1; /* Yeah it's a goto... deal with it */
    }   

#ifdef _DEBUG_
    char string[255];
    sprintf(string, "interface: %d -> %s", selector, netInterfaces[selector]);
    newtWinMessage(NULL, MSG_BUTTON_OK, string);
#endif
    /* We reset the selector to zero so we can reuse the variable */
    selector = 0;

    question2:
    returnValue = newtWinMenu(MSG_TITLE_NETWORK_SETTINGS, MSG_NETWORK_SETTINGS_INTERNAL_IF, 50, 5, 5, 3, 
		     netInterfaces, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->interfaceInternal = netInterfaces[selector];
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP);
            goto question2; /* Yeah it's a goto... deal with it */
    }

#ifdef _DEBUG_
    //char string[255];
    sprintf(string, "interface: %d -> %s", selector, netInterfaces[selector]);
    newtWinMessage(NULL, MSG_BUTTON_OK, string);
#endif

    char *entries[10];
    struct newtWinEntry managementNetworkEntries[] = {
        { "Management Network", entries + 0, 0 },
        { "Headnode IP", entries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question3:
    returnValue = newtWinEntries(
        MSG_TITLE_NETWORK_SETTINGS, MSG_NETWORK_SETTINGS_INTERNAL_IPV4, 50, 5, 5, 20,
        managementNetworkEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            /* Some password check logic should be implemented here*/
            if (tuiHasEmptyField(managementNetworkEntries))
                goto question3;
#ifdef _DEBUG_
            tuiDebugEntries(managementNetworkEntries);
#endif
            cluster->interfaceInternalNetwork = strdup(*managementNetworkEntries[0].value);
            cluster->interfaceInternalIP = strdup(*managementNetworkEntries[1].value);

            /* Clanup the memory */
            for (unsigned i = 0; i < sizeof (managementNetworkEntries) / sizeof (struct newtWinEntry) && managementNetworkEntries[i].value; i++)
                free(*(managementNetworkEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
            goto question3; /* Yeah it's a goto... deal with it */
    }

    struct newtWinEntry xCATDynamicRange[] = {
        { "Start IP", entries + 0, 0 },
        { "End IP", entries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question4:
    returnValue = newtWinEntries(
        MSG_TITLE_NETWORK_SETTINGS, MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE, 50, 5, 5, 20,
        xCATDynamicRange, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            /* Some password check logic should be implemented here*/
            if (tuiHasEmptyField(xCATDynamicRange))
                goto question4;
#ifdef _DEBUG_
            tuiDebugEntries(xCATDynamicRange);
#endif
            cluster->xCATDynamicRangeStart = strdup(*xCATDynamicRange[0].value);
            cluster->xCATDynamicRangeEnd = strdup(*xCATDynamicRange[1].value);

            /* Clanup the memory */
            for (unsigned i = 0; i < sizeof (xCATDynamicRange) / sizeof (struct newtWinEntry) && xCATDynamicRange[i].value; i++)
                free(*(xCATDynamicRange[i].value));

            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE_HELP);
            goto question3; /* Yeah it's a goto... deal with it */
    }

}

void tuiDirectoryServicesSettings(CLUSTER *cluster) {
    int returnValue;

    /* FreeIPA related settings */
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "FreeIPA admin password", entries + 0, NEWT_FLAG_PASSWORD },
        { "FreeIPA directory manager password", entries + 1, NEWT_FLAG_PASSWORD },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_DIRECTORY_SERVICES_SETTINGS, MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD, 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            /* Some password check logic should be implemented here*/
            if (tuiHasEmptyField(autoEntries))
                goto question0;
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->directoryAdminPassword = strdup(*autoEntries[0].value);
            cluster->directoryManagerPassword = strdup(*autoEntries[1].value);

            /* Clanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }

    question1:
    returnValue = newtWinTernary(
        MSG_TITLE_DIRECTORY_SERVICES_SETTINGS,
        MSG_BUTTON_YES, MSG_BUTTON_NO, MSG_BUTTON_HELP,
        MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC,
        NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->directoryDisableDNSSEC = true;
            break;
        case 2:
            cluster->directoryDisableDNSSEC = false;
            break;
        case 3:
            tuiHelpMessage(MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC_HELP);
            goto question1; /* Yeah it's a goto... deal with it */
    }

}

void tuiNodeSettings(CLUSTER *cluster) {
    int returnValue;

    /* UPDATE DOC HERE */
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "Prefix", entries + 0, 0 },
        { "Padding", entries + 1, 0 },
        { "Compute node first IP", entries + 2, 0 },
        { "Compute node root password", entries + 3, NEWT_FLAG_PASSWORD},
        { "ISO path of Node OS", entries + 4, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_NETWORK_SETTINGS, MSG_NODE_SETTINGS, 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(autoEntries))
                goto question0;

            if (isalpha(*autoEntries[0].value[0]) == false) {
                newtWinMessage(NULL, MSG_BUTTON_OK, "Prefix must start with a letter");
                goto question0;
            }
            /* This check is too complex and freaks up the pointers
             * | autoentries[1].value  -> char**
             * | (*autoentries[1].value) -> char*
             * | *(*autoentries[1].value)++ -> char
             */
            /*while (*(*autoEntries[1].value)) {
                if (isdigit(*(*autoEntries[1].value)++) == false) {
                    newtWinMessage(NULL, MSG_BUTTON_OK, "Padding can only have numbers");
                    goto question0;
                }
            }*/
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->nodePrefix = strdup(*autoEntries[0].value);
            cluster->nodePadding = strdup(*autoEntries[1].value);
            cluster->nodeStartIP = strdup(*autoEntries[2].value);
            cluster->nodeRootPassword = strdup(*autoEntries[3].value);
            cluster->nodeISOPath = strdup(*autoEntries[4].value);

            /* Cleanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_NODE_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

/* To be implemented
 * Here is another case where IPv4 settings are required for IPoIB specifically 
 */
void tuiInfinibandSettings(CLUSTER *cluster) {
    int returnValue;
    int selector = 0;

    question0:    
    returnValue = newtWinMenu(MSG_TITLE_INFINIBAND_SETTINGS, MSG_INFINIBAND_SETTINGS, 50, 5, 5, 3,
                    (char**) ibStacks, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->ibStack = strdup(ibStacks[selector]);
            
            if (selector) {
                tuiIPv4Settings(cluster);
            }
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_INFINIBAND_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

/* Not implemented */
void tuiIPv4Settings(CLUSTER *cluster) {
    /* This is just an ideia, but here we should as for the following:
     * . IPv4 address on the interface
     * . Network mask
     * . Physical interface?
     * . First node address? Where should compute nodes start - NOT A GOOD IDEA
     * . Generify network? Array of networks? Typedef enum networks?
     */
    int returnValue;

    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "Address", entries + 0, 0 },
        { "Subnet Mask", entries + 1, 0 },
        { "Gateway", entries + 2, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        "IP Configuration", "Fill the IP network information", 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(autoEntries))
                goto question0;
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->service.ip.address = strtoul((*autoEntries[0].value), NULL, 0);
            cluster->service.ip.netmask = strtoul((*autoEntries[1].value), NULL, 0);
            cluster->service.ip.gateway = strtoul((*autoEntries[2].value), NULL, 0);
                        
            /* Cleanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_POSTFIX_SASL_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

void tuiQueueSystemSettings(CLUSTER *cluster) {
    int returnValue;
    int selector = 0;

    question0:
    returnValue = newtWinMenu(MSG_TITLE_QUEUE_SYSTEM_SETTINGS, MSG_QUEUE_SYSTEM_SETTINGS, 50, 5, 5, 3,
                    (char**) queueSystems, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->queueSystem.name = strdup(queueSystems[selector]);           
#ifdef _DEBUG_
            //char string[255];
            //sprintf(string, "queueSystem: %u\n", cluster->queueSystem);
            //newtWinMessage(NULL, MSG_BUTTON_OK, string);
#endif
            /* I didn't like this implementation... perhaps a typedef enum would
             * do the job to get a better code
             */
            if (!selector)
                break;
            if (selector == 1) {
                tuiSLURMSettings(cluster);
                break;
            }
            if (selector == 2) {
                tuiPBSSettings(cluster);
                break;
            }
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_QUEUE_SYSTEM_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }

}

void tuiSLURMSettings(CLUSTER *cluster) {
    int returnValue;

    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "Partition name", entries + 0, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_SLURM_SETTINGS, MSG_SLURM_SETTINGS, 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(autoEntries))
                goto question0;
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->queueSystem.slurm.partition = strdup(*autoEntries[0].value);

            /* Clanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_SLURM_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

/* To be implemented
 * We need a global variable with PBS settings again; perhaps a typedef...
 * I need to think about the implementation
 */
void tuiPBSSettings(CLUSTER *cluster) {
    int returnValue;
    int selector = 0;

    question0:
    returnValue = newtWinMenu(MSG_TITLE_PBS_SETTINGS, MSG_PBS_SETTINGS, 50, 5, 5, 3,
                    (char**) pbsDefaultPlace, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->queueSystem.pbs.defaultPlace = strdup(pbsDefaultPlace[selector]);
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_PBS_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

void tuiPostfixSettings(CLUSTER *cluster) {
    int returnValue;

    question0:
    returnValue = newtWinTernary(
        MSG_TITLE_POSTFIX_SETTINGS,
        MSG_BUTTON_YES, MSG_BUTTON_NO, MSG_BUTTON_HELP,
        MSG_POSTFIX_ENABLE,
        NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->postfix.enable = true;
            break;
        case 2:
            cluster->postfix.enable = false;
            return;
            break;
        case 3:
            tuiHelpMessage(MSG_POSTFIX_ENABLE_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }

    int selector = 0;

    question1:
    returnValue = newtWinMenu(MSG_TITLE_POSTFIX_SETTINGS, MSG_POSTFIX_SETTINGS_PROFILE, 50, 5, 5, 3,
                    (char**) postfixProfiles, &selector, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);


    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->postfix.profileId = selector;
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_POSTFIX_SETTINGS_PROFILE_HELP);
            goto question1; /* Yeah it's a goto... deal with it */
    }

    /* This is not good... typedef enum? */
    if (cluster->postfix.profileId == 1)
        tuiPostfixRelaySettings(cluster);
    if (cluster->postfix.profileId == 2)
        tuiPostfixSASLSettings(cluster);

}

void tuiPostfixRelaySettings(CLUSTER *cluster) {
    int returnValue;

    /* UPDATE DOC HERE */
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "Hostname of the MTA", entries + 0, 0 },
        { "Port", entries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_POSTFIX_SETTINGS, MSG_POSTFIX_RELAY_SETTINGS, 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(autoEntries))
                goto question0;
            /* We need to check for correctness here, the code under us is just
             * an example, it's not what the function should do
             */
#if 0
            if (isalpha(*autoEntries[0].value[0]) == false) {
                newtWinMessage(NULL, MSG_BUTTON_OK, "Prefix must start with a letter");
                goto question0;
            }
#endif
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->postfix.relay.hostname = strdup(*autoEntries[0].value);
            cluster->postfix.relay.port = strtoul((*autoEntries[1].value), NULL, 0);
            
            /* Cleanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_POSTFIX_RELAY_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

void tuiPostfixSASLSettings(CLUSTER *cluster) {
    int returnValue;

    /* UPDATE DOC HERE */
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { "Hostname of the MTA", entries + 0, 0 },
        { "Port", entries + 1, 0 },
        { "Username", entries + 2, 0 },
        { "Password", entries + 3, NEWT_FLAG_PASSWORD },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    question0: 
    returnValue = newtWinEntries(
        MSG_TITLE_POSTFIX_SETTINGS, MSG_POSTFIX_SASL_SETTINGS, 50, 5, 5, 20,
        autoEntries, MSG_BUTTON_OK, MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (tuiHasEmptyField(autoEntries))
                goto question0;
            /* We need to check for correctness here, the code under us is just
             * an example, it's not what the function should do
             */
#if 0
            if (isalpha(*autoEntries[0].value[0]) == false) {
                newtWinMessage(NULL, MSG_BUTTON_OK, "Prefix must start with a letter");
                goto question0;
            }
#endif
#ifdef _DEBUG_
            tuiDebugEntries(autoEntries);
#endif
            cluster->postfix.sasl.hostname = strdup(*autoEntries[0].value);
            cluster->postfix.sasl.port = strtoul((*autoEntries[1].value), NULL, 0);
            cluster->postfix.sasl.username = strdup(*autoEntries[2].value);
            cluster->postfix.sasl.password = strdup(*autoEntries[3].value);
                        
            /* Cleanup the memory */
            for (unsigned i = 0; i < sizeof (autoEntries) / sizeof (struct newtWinEntry) && autoEntries[i].value; i++)
                free(*(autoEntries[i].value));
            break;
        case 2:
            tuiAbortInstall();
        case 3:
            tuiHelpMessage(MSG_POSTFIX_SASL_SETTINGS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }
}

void tuiUpdateSystem(CLUSTER *cluster) {
    int returnValue;
    
    question0:
    returnValue = newtWinTernary(
        MSG_TITLE_UPDATE_SYSTEM,
        MSG_BUTTON_YES, MSG_BUTTON_NO, MSG_BUTTON_HELP,
        MSG_UPDATE_SYSTEM,
        NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->updateSystem = true;
            break;
        case 2:
            cluster->updateSystem = false;
            break;
        case 3:
            tuiHelpMessage(MSG_UPDATE_SYSTEM_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }    
}

void tuiRemoteAccessSettings(CLUSTER *cluster) {
    int returnValue;
    
    question0:
    returnValue = newtWinTernary(
        MSG_TITLE_REMOTE_ACCESS,
        MSG_BUTTON_YES, MSG_BUTTON_NO, MSG_BUTTON_HELP,
        MSG_REMOTE_ACCESS,
        NULL
    );

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            cluster->remoteAccess = true;
            break;
        case 2:
            cluster->remoteAccess = false;
            break;
        case 3:
            tuiHelpMessage(MSG_REMOTE_ACCESS_HELP);
            goto question0; /* Yeah it's a goto... deal with it */
    }    
}

#ifdef _DEPRECATED_
/* This function is here as is, since it will not be used for now */
bool tuiCenteredMessage(void) {
    newtComponent form, text, button;
    struct newtExitStruct exitStatus;
    bool lumena = false;

    /* Screen size handling */
    int cols, rows;
    newtGetScreenSize(&cols, &rows);

    /* The number of coluns to be used for padding on both sides 
     * We usually let 2 columns in both sides to keep aesthetics
     */
    unsigned lateralPadding = 2;
    /* The lateral margin inside the textbox and the maximum allowed
     * textbox size considering the border of the window (1 column)
     */
    unsigned lateralMargin = 1;
    unsigned maxTextbox = cols - 2 * (lateralPadding + lateralMargin + 1);

    /* Prepare the textbox */
    text = newtTextboxReflowed(lateralMargin, 1, MSG_WELCOME, maxTextbox, 0, 0, 0);
    
    /* Button placement is calculated as follows:
     * We get the max textbox size inside the window, subtract the
     * length of the string inside the button, divide by two to find
     * the center of the textbox and finally subtract the border size 
     * of the button which is 1 column each side to fix the placement
     * The height of the box is just the number of lines from the
     * textbox plus two to get some space to the button.
     */
    char buttonText[] = "Lacra mais";
    button = newtButton((maxTextbox - strlen(buttonText)) / 2 - 2, newtTextboxGetNumLines(text) + 2, buttonText);
    
    /* Figure out the window width from the screen size considering
     * the define padding to keep visuals aesthetics. The height should
     * consider if we have a button or not. The default big button
     * needs 4 lines of height to be displayed. Considering the padding
     * for it a total of 6 lines must be provisioned for the button.
     */
    char *windowTitle = "Lumena";
    unsigned windowWidth = cols - 2 * (lateralPadding + 1);
    newtCenteredWindow(windowWidth, newtTextboxGetNumLines(text) + 6, windowTitle);

    /* Finally the form with the text */
    form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, text, button, NULL);

    //newtRunForm(form);

    newtFormRun(form, &exitStatus);

    if (exitStatus.reason == NEWT_EXIT_COMPONENT) {
        if (exitStatus.u.co == button) { // What a hell I'm doing???
            printf("Lumena autorizou\n");
            lumena = true;
        }
    }

    newtFormDestroy(form);

    /* Pop out the window we can print another one or exit
     */
    newtPopWindow();

    return lumena;

}
#endif

#ifdef _DEBUG_
void tuiDebugInfo(void) {
    /* Screen size handling */
    int cols, rows;
    newtGetScreenSize(&cols, &rows);

    char string[16];
    int size;
    newtComponent form, text, button;
    
    size = sprintf(string, "%d x %d", cols, rows);

    text = newtTextboxReflowed(1, 1, string, size, 0, 0, 0);
    button = newtButton((size / 2) + 1, newtTextboxGetNumLines(text) + 2, "Ok");

    char title[] = "Terminal Size";
    /* Padding to fix title is always 4 */
    /* Button has 4 lines including shadow */
    newtCenteredWindow(strlen(title) + 4, 8, title);

    form = newtForm(NULL, NULL, 0);
    newtFormAddComponents(form, text, button, NULL);
    
    /* newtFormRun(form,&exitStatus); should be used instead */
    newtRunForm(form);
    newtFormDestroy(form);

    newtPopWindow();
}
#endif

