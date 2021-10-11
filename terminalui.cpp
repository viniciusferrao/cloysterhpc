#include "terminalui.hpp"
#include "messages.hpp"
#include "headnode.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

/* Constructor */
TerminalUI::TerminalUI (Headnode *headnode) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();

    beginInstall(headnode);
}

/* Destructor */
TerminalUI::~TerminalUI (void) {
    newtFinished();
}

void TerminalUI::beginInstall (Headnode *headnode) {
//    drawWelcomeMessage();
//    drawWimeSettings(headnode);
//    drawLocaleSettings(headnode);
    drawNetworkSettings(headnode);
#if 0
    directoryServicesSettings(cluster);
    nodeSettings(cluster);
    infinibandSettings(cluster);
    queueSystemSettings(cluster);
    postfixSettings(cluster);
    updateSystem(cluster);
    remoteAccessSettings(cluster);
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

std::string TerminalUI::drawListMenu (const char * title, const char * message,
                                      const char * const * items,
                                      const char * helpMessage) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3; 

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
    const char * helpMessage
) {

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
            tuiDebugEntries(hostId);
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
void TerminalUI::drawNetworkSettings (Headnode *headnode) {
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

    headnode->interfaceExternal = drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
                                      MSG_NETWORK_SETTINGS_EXTERNAL_IF, netInterfaces,
                                      MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);

    headnode->interfaceInternal = drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
                                      MSG_NETWORK_SETTINGS_INTERNAL_IF, netInterfaces,
                                      MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP);
 
#if 0
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
#endif /* #if 0 */
}
