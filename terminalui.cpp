#include "terminalui.hpp"
#include "messages.hpp"
#include "headnode.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

/* Constructor */
TerminalUI::TerminalUI (Cluster *cluster, Headnode *headnode) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();

    beginInstall(cluster, headnode);
}

/* Destructor */
TerminalUI::~TerminalUI (void) {
    newtFinished();
}

void TerminalUI::beginInstall (Cluster *cluster, Headnode *headnode) {
#if 0
    drawWelcomeMessage();
    drawWimeSettings(headnode);
    drawLocaleSettings(headnode);
    drawNetworkSettings(headnode);
    drawDirectoryServicesSettings(cluster);
#endif
    drawNodeSettings(cluster);
#if 0
    drawInfinibandSettings(cluster);
    drawQueueSystemSettings(cluster);
    drawPostfixSettings(cluster);
    drawUpdateSystem(cluster);
    drawRemoteAccessSettings(cluster);
#endif
}

void TerminalUI::abortInstall (void) {
    TerminalUI::~TerminalUI();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

/* Refactor this with a C++ dialect */
bool TerminalUI::hasEmptyField (const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), message);
            return true;
        }
    }

    return false;
}

bool TerminalUI::drawYesNoQuestion (const char * title, const char * message, 
                                    const char * helpMessage) {
    int returnValue;

    question:
    returnValue = newtWinTernary(const_cast<char *>(title), 
                                 const_cast<char *>(MSG_BUTTON_YES),
                                 const_cast<char *>(MSG_BUTTON_NO),
                                 const_cast<char *>(MSG_BUTTON_HELP),
                                 const_cast<char *>(message), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return true;
        case 2:
            return false;
        case 3:
            drawHelpMessage(helpMessage);
            goto question; /* Yeah it's a goto... deal with it */
    }
    return false; /* We should never reach here */
}

std::string TerminalUI::drawListMenu (const char * title, const char * message,
                                      const char * const * items,
                                      const char * helpMessage) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3; 
#if 1
    /* Goto implementation */
    question:
    returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(items), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL), 
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return items[selector];
        case 2:
            abortInstall();
        case 3:
            drawHelpMessage(helpMessage);
            goto question;
    }
#endif
#if 0
    /* Gotoless implementation */
    for (;;) {
        returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(items), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL), 
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return items[selector];
            case 2:
                abortInstall();
            case 3:
                drawHelpMessage(helpMessage);
                continue;
        }
        break; /* for (;;) */
    }
#endif
    return "Error!"; /* We should never reach here */
}

std::vector<std::string> TerminalUI::drawFieldMenu (
    const char * title, const char * message, struct newtWinEntry * items, 
    const char * helpMessage) {

    int returnValue;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 20; 

    std::vector<std::string> fields;

    question: 
    returnValue = newtWinEntries(const_cast<char *>(title), 
                                 const_cast<char *>(message),
                                 suggestedWidth, flexUp, flexDown,
                                 maxHeightList, items, 
                                 const_cast<char *>(MSG_BUTTON_OK), 
                                 MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (hasEmptyField(items))
                goto question;      
#ifdef _DEBUG_
            //tuiDebugEntries(hostId);
#endif
            /* Cleanup the memory for items: that's a funny thing here, our
             * friend newtWinEntries() it's not really here to free hostId but 
             * to populate it with data. So we need to clean this data after
             * using it.
             */
            for (unsigned i = 0 ; items[i].value ; i++) {
                     fields.push_back(*items[i].value);
                     free(*(items[i].value));
            }

            return fields;
        case 2:
            abortInstall();
        case 3:
            drawHelpMessage(helpMessage);
            goto question;
    }
    return fields; /* We should never reach here */
}

/* This is the default help action function, that should be called whatever the
 * operator asks for help.
 * The nature of the function relies on const char* for safety reasons but the
 * underling function requires char *; so we basically need to const_cast the
 * constexpr string just to display the message.
 */
void TerminalUI::drawHelpMessage (const char* message) {
    newtBell();
    newtWinMessage(const_cast<char *>(MSG_TITLE_HELP),
                   const_cast<char *>(MSG_BUTTON_OK),
                   const_cast<char *>(message));
}

void TerminalUI::drawWelcomeMessage (void) {
    int returnValue;

    /* We start pushing the welcome message */
    newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), 
                   const_cast<char *>(MSG_WELCOME));

    /* Information about the installation scheme */
    returnValue = newtWinChoice(NULL, const_cast<char *>(MSG_BUTTON_OK), 
                                const_cast<char *>(MSG_BUTTON_CANCEL),
                                const_cast<char *>(MSG_GUIDED_INSTALL));

    switch (returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            break;
        case 2:
            abortInstall();
    }
}

void TerminalUI::drawTimeSettings (Headnode *headnode) {
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

    headnode->timezone = drawListMenu(MSG_TITLE_TIME_SETTINGS,
                                      MSG_TIME_SETTINGS_TIMEZONE, timezones,
                                      MSG_TIME_SETTINGS_TIMEZONE_HELP);
}

void TerminalUI::drawLocaleSettings (Headnode *headnode) {
    /* This is a placeholder const until we figure out how to fetch the list of
     * supported locales from the OS instead.
     */
    const char* const locales[] = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C",
        NULL
    };

    headnode->timezone = drawListMenu(MSG_TITLE_LOCALE_SETTINGS,
                                      MSG_LOCALE_SETTINGS_LOCALE, locales,
                                      MSG_LOCALE_SETTINGS_LOCALE_HELP);
}

/* We ask the required networking info to provide the cluster
 * This function is not that good, the data modelling isn't good either. Perhaps
 * we should have a function to handle IPv4 questions and call it when needed.
 * Questions about network are much the same on the service and application
 * networks, so it's duplicated code. xCAT does not have a good fit here too, it
 * should be modular so we can remove xCAT if we need to.
 */
void TerminalUI::drawNetworkSettings (Cluster *cluster, 
                                      Headnode *headnode) {

    drawNetworkHostnameSettings(headnode);
    drawNetworkExternalInterfaceSelection(headnode);
    drawNetworkManagementInterfaceSelection(headnode);
    drawNetworkManagementAddress(headnode);
    drawNetworkManagementXCATRange(cluster);
}

void TerminalUI::drawNetworkHostnameSettings (Headnode *headnode) {
    /* Request hostname and domain name */
    char *hostIdEntries[2];
    struct newtWinEntry hostId[] = {
        { const_cast<char *>("Hostname"), hostIdEntries + 0, 0 },
        { const_cast<char *>("Domain name"), hostIdEntries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(hostIdEntries, 0, sizeof(hostIdEntries));

    std::vector<std::string> fields = drawFieldMenu(
                                        MSG_TITLE_NETWORK_SETTINGS,
                                        MSG_NETWORK_SETTINGS_HOSTID, hostId,
                                        MSG_NETWORK_SETTINGS_HOSTID_HELP);

    headnode->hostname = fields[0];
    headnode->domainname = fields[1];
    headnode->fqdn = headnode->hostname + "." + headnode->domainname;

#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif

}

void TerminalUI::drawNetworkExternalInterfaceSelection (Headnode *headnode) {
    //char **netInterfaces;
    /* Implement with https://linux.die.net/man/3/getifaddrs */
    const char* const netInterfaces[] = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0",
        NULL
    };

    /* This is totally wrong, it should set the interface name and not the IP
     * address of the interface
     */
    Network network;
    network.setProfile(Network::Profile::External);
    network.setType(Network::Type::Ethernet);
    //network.setIPAddress(drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
    drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
                     MSG_NETWORK_SETTINGS_EXTERNAL_IF, netInterfaces,
                     MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);

    headnode->externalNetwork.push_back(network);
}

void TerminalUI::drawNetworkManagementInterfaceSelection (Headnode *headnode) {
    //char **netInterfaces;
    /* Implement with https://linux.die.net/man/3/getifaddrs */
    const char* const netInterfaces[] = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0",
        NULL
    };

    /* This is totally wrong, it should set the interface name and not the IP
     * address of the interface
     */
    Network network;
    network.setProfile(Network::Profile::Management);
    network.setType(Network::Type::Ethernet);
    //network.setIPAddress(drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
    drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
                    MSG_NETWORK_SETTINGS_INTERNAL_IF, netInterfaces,
                    MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP);

    headnode->managementNetwork.push_back(network);
}

void TerminalUI::drawNetworkManagementAddress (Headnode *headnode) {
    char *entries[10];
    struct newtWinEntry managementNetworkEntries[] = {
        { const_cast<char *>("Headnode IP"), entries + 0, 0 },
        { const_cast<char *>("Management Network"), entries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_NETWORK_SETTINGS,
                                    MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                                    managementNetworkEntries,
                                    MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);

    headnode->managementNetwork[0].setIPAddress(fields[0], fields[1]);

#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif
}

void TerminalUI::drawNetworkManagementXCATRange (Cluster *cluster) {
    char *entries[10];
    struct newtWinEntry xCATDynamicRange[] = {
        { const_cast<char *>("Start IP"), entries + 0, 0 },
        { const_cast<char *>("End IP"), entries + 1, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_NETWORK_SETTINGS,
                                    MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE,
                                    xCATDynamicRange,
                                    MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE_HELP);

    cluster->xCATDynamicRangeStart = fields[0];
    cluster->xCATDynamicRangeEnd = fields[1];

#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif
}

void TerminalUI::drawDirectoryServicesSettings (Cluster *cluster) {
    drawDirectoryServicesPassword(cluster);
    drawDirectoryServicesDisableDNSSEC(cluster);
}

void TerminalUI::drawDirectoryServicesPassword (Cluster *cluster) {
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { const_cast<char *>("FreeIPA admin password"), 
            entries + 0, NEWT_FLAG_PASSWORD },
        { const_cast<char *>("FreeIPA directory manager password"), 
            entries + 1, NEWT_FLAG_PASSWORD },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    std::vector<std::string> fields = drawFieldMenu(
                                MSG_TITLE_DIRECTORY_SERVICES_SETTINGS,
                                MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD,
                                autoEntries,
                                MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD_HELP);

    cluster->directoryAdminPassword = fields[0];
    cluster->directoryManagerPassword = fields[1];
}

void TerminalUI::drawDirectoryServicesDisableDNSSEC (Cluster *cluster) {
    const bool disableDNSSEC = drawYesNoQuestion(
                                MSG_TITLE_DIRECTORY_SERVICES_SETTINGS,
                                MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC,
                                MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC_HELP);

    disableDNSSEC ? 
        cluster->directoryDisableDNSSEC = true : 
        cluster->directoryDisableDNSSEC = false;
}

void TerminalUI::drawNodeSettings (Cluster *cluster) {
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { const_cast<char *>("Prefix"), entries + 0, 0 },
        { const_cast<char *>("Padding"), entries + 1, 0 },
        { const_cast<char *>("Compute node first IP"), entries + 2, 0 },
        { const_cast<char *>("Compute node root password"), 
            entries + 3, NEWT_FLAG_PASSWORD},
        { const_cast<char *>("ISO path of Node OS"), entries + 4, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    retry:
    std::vector<std::string> fields = drawFieldMenu(
                                MSG_TITLE_NETWORK_SETTINGS,
                                MSG_NODE_SETTINGS,
                                autoEntries,
                                MSG_NODE_SETTINGS_HELP);

    /* TODO: Better implementation */
    if (isalpha(fields[0][0]) == false) {
        newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), 
                       const_cast<char *>("Prefix must start with a letter"));
        memset(entries, 0, sizeof(entries));
        goto retry;
    }

    cluster->nodePrefix = fields[0];
    cluster->nodePadding = fields[1];
    cluster->nodeStartIP = fields[2];
    cluster->nodeRootPassword = fields[3];
    cluster->nodeISOPath = fields[4];
}

void TerminalUI::drawQueueSystemSelection (Cluster *cluster) {
    const char* const queueSystems[] = {
        "None",
        "SLURM",
        "PBS Professional",
        NULL
    };

    std::string queueSystem = drawListMenu(
        MSG_TITLE_QUEUE_SYSTEM_SETTINGS, MSG_QUEUE_SYSTEM_SETTINGS,
        queueSystems, MSG_QUEUE_SYSTEM_SETTINGS_HELP);

    cluster->queueSystem.name = queueSystem;

    if (queueSystem == "SLURM") {
        drawSLURMSettings(cluster);
        return;
    }

    if (queueSystem == "PBS Professional") {
        drawPBSSettings(cluster);
        return;
    }
}

void TerminalUI::drawSLURMSettings (Cluster *cluster) {
    char *entries[10];
    struct newtWinEntry autoEntries[] = {
        { const_cast<char *>("Partition name"), entries + 0, 0 },
        { NULL, NULL, 0 } };
    memset(entries, 0, sizeof(entries));

    const std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_SLURM_SETTINGS,
                                    MSG_SLURM_SETTINGS,
                                    autoEntries,
                                    MSG_SLURM_SETTINGS_HELP);

    cluster->queueSystem.slurm = { fields[0] };
}

void TerminalUI::drawPBSSettings (Cluster *cluster) {
    const char* const pbsDefaultPlace[] = {
        "Shared",
        "Scatter",
        NULL
    };

    const std::string defaultPlace = drawListMenu(MSG_TITLE_PBS_SETTINGS,
                                      MSG_PBS_SETTINGS, 
                                      pbsDefaultPlace,
                                      MSG_PBS_SETTINGS_HELP);

    if (defaultPlace == "Shared")
        cluster->queueSystem.pbs = { PBS::DefaultPlace::Shared };
    if (defaultPlace == "Scatter")
        cluster->queueSystem.pbs = { PBS::DefaultPlace::Scatter };

}
