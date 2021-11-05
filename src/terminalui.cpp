#include "terminalui.h"
#include "messages.h"
#include "headnode.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstring> /* strlen() */

#include <fmt/format.h>

/* Constructor */
TerminalUI::TerminalUI (Cluster& cluster, Headnode& headnode) {
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
TerminalUI::~TerminalUI () {
    newtFinished();
}

void TerminalUI::beginInstall (Cluster& cluster, Headnode& headnode) {
#if 1
    drawWelcomeMessage();
    drawTimeSettings(cluster);
    drawLocaleSettings(cluster);
    drawNetworkSettings(cluster, headnode);
    drawInfinibandSettings(cluster);
    drawDirectoryServicesSettings(cluster);
    drawNodeSettings(cluster);
    drawQueueSystemSettings(cluster);
    drawPostfixSettings(cluster);
    drawUpdateSystem(cluster);
    drawRemoteAccess(cluster);
#endif
}

void TerminalUI::abortInstall () {
    TerminalUI::~TerminalUI();
    std::cout << MSG_INSTALL_ABORT;
    exit(EXIT_SUCCESS);
}

/* Refactor this with a C++ dialect */
bool TerminalUI::hasEmptyField (const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries, and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(nullptr, const_cast<char *>(MSG_BUTTON_OK), message);
            return true;
        }
    }

    return false;
}

/* Refactor this with a C++ dialect */
#ifdef _DEBUG_
void TerminalUI::drawDebugEntries(const struct newtWinEntry *entries) {
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
    
    newtWinMessage(NULL, const_cast<char *>(MSG_BUTTON_OK), message);
}
#endif

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
                                      const std::vector<std::string>& items,
                                      const char * helpMessage) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3;

    /* Newt expects a NULL terminated array of C style strings */
    std::vector<const char*> cStrings;
    cStrings.reserve(items.size() + 1);

    for (const auto& item : items) {
        cStrings.push_back(item.c_str());
    }
    cStrings.push_back(nullptr);

#if 1
    /* Goto implementation */
    question:
    returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexDown, flexUp, maxHeightList,
                              const_cast<char **>(cStrings.data()), &selector,
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
    const char * title, const char * message,
    const std::vector<std::string>& items, const char * helpMessage) {

    int returnValue;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 20;

    char** fieldEntries;
    struct newtWinEntry* field;
    unsigned vectorSize = items.size();

    field = new struct newtWinEntry[vectorSize + 1]();
    fieldEntries = new char*[vectorSize + 1]();

    for (unsigned i = 0 ; i < vectorSize ; i++) {
        field[i].text = const_cast<char*>(items[i].c_str());
        field[i].value = fieldEntries + i;
        /* TODO: Fix this hack to enable password fields */
        if (items[i].find("Password") != std::string::npos ||
            items[i].find("password") != std::string::npos)
            field[i].flags = NEWT_FLAG_PASSWORD;
        else
            field[i].flags = 0;
    }
    field[vectorSize].text = nullptr;
    field[vectorSize].value = nullptr;
    field[vectorSize].flags = 0;

    std::vector<std::string> entries;

    question: 
    returnValue = newtWinEntries(const_cast<char *>(title), 
                                 const_cast<char *>(message),
                                 suggestedWidth, flexDown, flexUp,
                                 maxHeightList, field,
                                 const_cast<char *>(MSG_BUTTON_OK), 
                                 MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (hasEmptyField(field))
                goto question;      
#ifdef _DEBUG_
            drawDebugEntries(field);
#endif
            for (unsigned i = 0 ; field[i].value ; i++) {
                entries.push_back(*field[i].value);
            }

            delete[] fieldEntries;
            delete[] field;

            return entries;
        case 2:
            abortInstall();
        case 3:
            drawHelpMessage(helpMessage);
            goto question;
        default:
            throw;
    }
}

/* This is just an ideia, but here we should do the following:
 * . IPv4 address on the interface
 * . Network mask
 * . Physical interface confirmation on screen
 * . Make network generic? Vector of networks? enum class networks?
 * . Call drawFieldMenu instead since it's almost the same code, the only change
 * is that we don't pass the newtWinEntry structure, because it's fixed.
 */
std::vector<std::string> TerminalUI::drawIPSettings (const char * title, 
                                        const char * message, 
                                        const char * helpMessage/*, 
                                        Network *network*/) {
    int returnValue;

    char *entries[10];
    struct newtWinEntry items[] = {
        { const_cast<char *>("Address"), entries + 0, 0 },
        { const_cast<char *>("Subnet Mask"), entries + 1, 0 },
        { const_cast<char *>("Gateway"), entries + 2, 0 },
        { nullptr, nullptr, 0 } };
    memset(items, 0, sizeof(items));

    std::vector<std::string> fields;

    question: 
    returnValue = newtWinEntries(
        const_cast<char *>(title),
        const_cast<char *>(message),
        50, 5, 5, 20, items,
        const_cast<char *>(MSG_BUTTON_OK), 
        const_cast<char *>(MSG_BUTTON_CANCEL), 
        const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (hasEmptyField(items))
                goto question;
#ifdef _DEBUG_
            drawDebugEntries(items);
#endif
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

void TerminalUI::drawWelcomeMessage () {
    int returnValue;

    /* We start pushing the welcome message */
    newtWinMessage(nullptr, const_cast<char *>(MSG_BUTTON_OK),
                   const_cast<char *>(MSG_WELCOME));

    /* Information about the installation scheme */
    returnValue = newtWinChoice(nullptr, const_cast<char *>(MSG_BUTTON_OK),
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

void TerminalUI::drawTimeSettings (Cluster& cluster) {
    /* This is a placeholder const until we figure out how to fetch the list of
     * supported timezones from the OS instead.
     */
    // std::chrono::tzdb_list& get_tzdb_list();
    // std::vector<std::string> timezones;
    // for (auto x : tzdb_list) {
    //     timezones.push_back(x);
    // }
    std::vector<std::string> timezones = {
        "America/Sao_Paulo",
        "UTC",
        "Gadific Mean Bolsotime",
        "Chronus",
        "Two blocks ahead"
    };

    cluster.setTimezone(drawListMenu(MSG_TITLE_TIME_SETTINGS,
                                     MSG_TIME_SETTINGS_TIMEZONE, timezones,
                                     MSG_TIME_SETTINGS_TIMEZONE_HELP));
}

void TerminalUI::drawLocaleSettings (Cluster& cluster) {
    /* This is a placeholder const until we figure out how to fetch the list of
     * supported locales from the OS instead.
     */
    std::vector<std::string> locales = {
        "en.US_UTF-8",
        "pt.BR_UTF-8",
        "C"
    };

    cluster.setLocale(drawListMenu(MSG_TITLE_LOCALE_SETTINGS,
                                   MSG_LOCALE_SETTINGS_LOCALE, locales,
                                   MSG_LOCALE_SETTINGS_LOCALE_HELP));
}

/* We ask the required networking info to provide the cluster
 * This function is not that good, the data modelling isn't good either. Perhaps
 * we should have a function to handle IPv4 questions and call it when needed.
 * Questions about network are much the same on the service and application
 * networks, so it's duplicated code. xCAT does not have a good fit here too, it
 * should be an interface (modular), so we can remove xCAT if we need to.
 */
void TerminalUI::drawNetworkSettings (Cluster& cluster, 
                                      Headnode& headnode) {

    drawNetworkHostnameSettings(cluster);
    drawNetworkExternalInterfaceSelection(headnode);
    drawNetworkManagementInterfaceSelection(headnode);
    drawNetworkManagementAddress(headnode);
    drawNetworkManagementXCATRange(cluster);
}

void TerminalUI::drawNetworkHostnameSettings (Cluster& cluster) {
    /* Request m_hostname and domain name */
    const std::vector<std::string> entries = {
            "Hostname",
            "Domain Name"
    };;

    std::vector<std::string> fields = drawFieldMenu(
                                        MSG_TITLE_NETWORK_SETTINGS,
                                        MSG_NETWORK_SETTINGS_HOSTID, entries,
                                        MSG_NETWORK_SETTINGS_HOSTID_HELP);
#if 0
    cluster.m_headnode->setHostname(fields[0]);
    cluster.setDomainName(fields[1]);
    cluster.m_headnode->setFQDN(fmt::format("{0}.{1}",
                                cluster.m_headnode->getHostname(),
                                cluster.getDomainName()));
#endif
#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif

}

void TerminalUI::drawNetworkExternalInterfaceSelection (Headnode& headnode) {
    //char **netInterfaces;
    /* Implement with https://linux.die.net/man/3/getifaddrs */
    std::vector<std::string> netInterfaces = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0",
    };

    Network network(Network::Profile::External,
                    Network::Type::Ethernet);
//    Connection connection;
//    connection.setInterfaceName(drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
//                     MSG_NETWORK_SETTINGS_EXTERNAL_IF, netInterfaces,
//                     MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP));
//
//    headnode.externalNetwork.push_back(network);
}

void TerminalUI::drawNetworkManagementInterfaceSelection (Headnode& headnode) {
    //char **netInterfaces;
    /* Implement with https://linux.die.net/man/3/getifaddrs */
    std::vector<std::string> netInterfaces = {
        "eth0",
        "eth1",
        "enp4s0f0",
        "lo",
        "ib0"
    };

    /* This is totally wrong, it should set the interface name and not the IP
     * address of the interface
     */
    Network network(Network::Profile::Management,
                    Network::Type::Ethernet);
//    Connection connection;
//    connection.setInterfaceName(drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
//                    MSG_NETWORK_SETTINGS_INTERNAL_IF, netInterfaces,
//                    MSG_NETWORK_SETTINGS_INTERNAL_IF_HELP));
//
//    headnode.managementNetwork.push_back(network);
}

void TerminalUI::drawNetworkManagementAddress (Headnode& headnode) {
    const std::vector<std::string> managementNetworkEntries = {
            "Headnode IP",
            "Management Network"
    };

    std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_NETWORK_SETTINGS,
                                    MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                                    managementNetworkEntries,
                                    MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);

    //headnode.managementNetwork[0].setIPAddress(fields[0], fields[1]);

#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif
}

void TerminalUI::drawNetworkManagementXCATRange (Cluster& cluster) {
    const std::vector<std::string> xCATDynamicRange = {
        "Start IP",
        "End IP"
    };

    std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_NETWORK_SETTINGS,
                                    MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE,
                                    xCATDynamicRange,
                                    MSG_NETWORK_SETTINGS_XCAT_DHCP_RANGE_HELP);

    cluster.xCATDynamicRangeStart = fields[0];
    cluster.xCATDynamicRangeEnd = fields[1];

#ifdef _DEBUG_
    std::cerr << "Strings on Vector: ";
    for (std::string i : fields) {
        std::cerr << i << ' ';
    }
    std::cerr << std::endl;
#endif
}

/* To be implemented
 * Here is another case where IPv4 settings are required for IPoIB specifically 
 */
void TerminalUI::drawInfinibandSettings (Cluster& cluster) {
    std::vector<std::string> ibStacks = {
        "None",
        "Inbox",
        "Mellanox"
    };
    
    cluster.ibStack = drawListMenu(MSG_TITLE_INFINIBAND_SETTINGS,
                                      MSG_INFINIBAND_SETTINGS, ibStacks,
                                      MSG_INFINIBAND_SETTINGS_HELP);

    /* We should ask for IPoIB over here or in another function */
    //drawIPSettings(application);
}

void TerminalUI::drawDirectoryServicesSettings (Cluster& cluster) {
    drawDirectoryServicesPassword(cluster);
    drawDirectoryServicesDisableDNSSEC(cluster);
}

void TerminalUI::drawDirectoryServicesPassword (Cluster& cluster) {
    const std::vector<std::string> entries = {
        "FreeIPA admin password",
        "FreeIPA directory manager password"
    };

    std::vector<std::string> fields = drawFieldMenu(
                                MSG_TITLE_DIRECTORY_SERVICES_SETTINGS,
                                MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD,
                                entries,
                                MSG_DIRECTORY_SERVICES_SETTINGS_PASSWORD_HELP);

    cluster.directoryAdminPassword = fields[0];
    cluster.directoryManagerPassword = fields[1];
}

void TerminalUI::drawDirectoryServicesDisableDNSSEC (Cluster& cluster) {
    const bool disableDNSSEC = drawYesNoQuestion(
                                MSG_TITLE_DIRECTORY_SERVICES_SETTINGS,
                                MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC,
                                MSG_DIRECTORY_SERVICES_SETTINGS_DNSSEC_HELP);

    disableDNSSEC ? 
        cluster.directoryDisableDNSSEC = true : 
        cluster.directoryDisableDNSSEC = false;
}

void TerminalUI::drawNodeSettings (Cluster& cluster) {
    const std::vector<std::string> entries = {
        "Prefix",
        "Padding",
        "Compute node first IP",
        "Compute node root password",
        "ISO path of Node OS"
    };

    retry:
    std::vector<std::string> fields = drawFieldMenu(
                                MSG_TITLE_NETWORK_SETTINGS,
                                MSG_NODE_SETTINGS,
                                entries,
                                MSG_NODE_SETTINGS_HELP);

    /* TODO: Better implementation, logic must be outside of the view */
    if (isalpha(fields[0][0]) == false) {
        newtWinMessage(nullptr, const_cast<char *>(MSG_BUTTON_OK),
                       const_cast<char *>("Prefix must start with a letter"));
        goto retry;
    }

    cluster.nodePrefix = fields[0];
    cluster.nodePadding = fields[1];
    cluster.nodeStartIP = fields[2];
    cluster.nodeRootPassword = fields[3];
    cluster.nodeISOPath = fields[4];
}

void TerminalUI::drawQueueSystemSettings (Cluster& cluster) {
    std::vector<std::string> queueSystems = {
        "None",
        "SLURM",
        "PBS Professional"
    };

    std::string queueSystem = drawListMenu(
        MSG_TITLE_QUEUE_SYSTEM_SETTINGS, MSG_QUEUE_SYSTEM_SETTINGS,
        queueSystems, MSG_QUEUE_SYSTEM_SETTINGS_HELP);

    cluster.queueSystem.name = queueSystem;

    if (queueSystem == "SLURM") {
        drawSLURMSettings(cluster);
        return;
    }

    if (queueSystem == "PBS Professional") {
        drawPBSSettings(cluster);
        return;
    }
}

void TerminalUI::drawSLURMSettings (Cluster& cluster) {
    const std::vector<std::string> entries = {
        "Partition name"
    };

    const std::vector<std::string> fields = drawFieldMenu(
                                    MSG_TITLE_SLURM_SETTINGS,
                                    MSG_SLURM_SETTINGS,
                                    entries,
                                    MSG_SLURM_SETTINGS_HELP);

    cluster.queueSystem.slurm = { fields[0] };
}

/* This function is broken since we still don't know how to use std::optional */
void TerminalUI::drawPBSSettings (Cluster& cluster) {
    std::vector<std::string> pbsDefaultPlace = {
        "Shared",
        "Scatter"
    };

    const std::string defaultPlace = drawListMenu(MSG_TITLE_PBS_SETTINGS,
                                      MSG_PBS_SETTINGS, 
                                      pbsDefaultPlace,
                                      MSG_PBS_SETTINGS_HELP);

    /* std::optional implementation is wrong */
#if 0
    if (defaultPlace == "Shared")
        cluster.queueSystem.pbs.defaultPlace = { PBS::DefaultPlace::Shared };
    if (defaultPlace == "Scatter")
        cluster.queueSystem.pbs.defaultPlace = { PBS::DefaultPlace::Scatter };
#endif

}

void TerminalUI::drawPostfixSettings (Cluster& cluster) {
    drawPostfixEnable(cluster);

    if (!cluster.postfix.enable)
        return;
    
    drawPostfixProfile(cluster);

    if (cluster.postfix.profileId == Postfix::ProfileId::Relay)
        drawPostfixRelaySettings(cluster);

    if (cluster.postfix.profileId == Postfix::ProfileId::SASL)
        drawPostfixSASLSettings(cluster);
}

void TerminalUI::drawPostfixEnable (Cluster& cluster) {
    const bool enablePostfix = drawYesNoQuestion(
                                MSG_TITLE_POSTFIX_SETTINGS,
                                MSG_POSTFIX_ENABLE,
                                MSG_POSTFIX_ENABLE_HELP);

    enablePostfix ? 
        cluster.postfix.enable = true : 
        cluster.postfix.enable = false;
}

void TerminalUI::drawPostfixProfile (Cluster& cluster) {
    std::vector<std::string> postfixProfiles = {
        "Local",
        "Relay",
        "SASL"
    };

    const std::string postfixProfile = drawListMenu(MSG_TITLE_POSTFIX_SETTINGS,
                                    MSG_POSTFIX_SETTINGS_PROFILE, 
                                    postfixProfiles,
                                    MSG_POSTFIX_SETTINGS_PROFILE_HELP);

    if (postfixProfile == "Local")
        return;
    if (postfixProfile == "Relay")
        cluster.postfix.profileId = Postfix::ProfileId::Relay;
    if (postfixProfile == "SASL")
        cluster.postfix.profileId = Postfix::ProfileId::SASL;
}

void TerminalUI::drawPostfixRelaySettings (Cluster& cluster) {
    const std::vector<std::string> entries = {
        "Hostname of the MTA",
        "Port"
    };

    std::vector<std::string> fields = drawFieldMenu(
                            MSG_TITLE_POSTFIX_SETTINGS,
                            MSG_POSTFIX_RELAY_SETTINGS,
                            entries,
                            MSG_POSTFIX_RELAY_SETTINGS_HELP);

    /* More std::optional shenanigans */
#if 0
    cluster.postfix.relay.m_hostname = fields[0];
    cluster.postfix.relay.port = fields[1];
#endif
}

void TerminalUI::drawPostfixSASLSettings (Cluster& cluster) {
    const std::vector<std::string> entries = {
        "Hostname of the MTA",
        "Port",
        "Username",
        "Password"
    };


    std::vector<std::string> fields = drawFieldMenu(
                            MSG_TITLE_POSTFIX_SETTINGS,
                            MSG_POSTFIX_SASL_SETTINGS,
                            entries,
                            MSG_POSTFIX_SASL_SETTINGS_HELP);

    /* More std::optional shenanigans */
#if 0
    cluster.postfix.sasl.m_hostname = fields[0];
    cluster.postfix.sasl.port = fields[1];
    cluster.postfix.sasl.username = fields[2];
    cluster.postfix.sasl.password = fields[3];
#endif
}

void TerminalUI::drawUpdateSystem (Cluster& cluster) {
    const bool updateSystem = drawYesNoQuestion(
                            MSG_TITLE_UPDATE_SYSTEM,
                            MSG_UPDATE_SYSTEM,
                            MSG_UPDATE_SYSTEM_HELP);

    updateSystem ? 
        cluster.updateSystem = true : 
        cluster.updateSystem = false;
}

void TerminalUI::drawRemoteAccess (Cluster& cluster) {
    const bool remoteAccess = drawYesNoQuestion(
                            MSG_TITLE_REMOTE_ACCESS,
                            MSG_REMOTE_ACCESS,
                            MSG_REMOTE_ACCESS_HELP);

    remoteAccess ? 
        cluster.remoteAccess = true : 
        cluster.remoteAccess = false;
}

/* Refactoring to MVC */
TerminalUI::TerminalUI () {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();
}

TerminalUI::TerminalUI (Cluster& cluster) {
    newtInit();
    newtCls();

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, MSG_INSTALL_TITLE);

    /* Add the default help line in the bottom */
    newtPushHelpLine(MSG_INSTALL_HELP_LINE);
    newtRefresh();

//    beginInstall(cluster);
//    void drawHelpMessage (const char *);
//    void drawWelcomeMessage ();
//    void drawTimeSettings (Headnode&);
//    void drawLocaleSettings (Headnode&);
//    void drawNetworkSettings (Cluster&, Headnode&);
//    void drawNetworkHostnameSettings (Headnode&);
//    void drawNetworkExternalInterfaceSelection (Headnode&);
//    void drawNetworkManagementInterfaceSelection (Headnode&);
//    void drawNetworkManagementAddress (Headnode&);
//    void drawNetworkManagementXCATRange (Cluster&);
//    void drawInfinibandSettings(Cluster&);
//    void drawDirectoryServicesSettings (Cluster&);
//    void drawDirectoryServicesPassword (Cluster&);
//    void drawDirectoryServicesDisableDNSSEC (Cluster&);
//    void drawNodeSettings (Cluster&);
//    void drawQueueSystemSettings (Cluster&);
//    void drawSLURMSettings (Cluster&);
//    void drawPBSSettings (Cluster&);
//    void drawPostfixSettings (Cluster&);
//    void drawPostfixEnable (Cluster&);
//    void drawPostfixProfile (Cluster&);
//    void drawPostfixRelaySettings (Cluster&);
//    void drawPostfixSASLSettings (Cluster&);
//    void drawUpdateSystem (Cluster&);
//    void drawRemoteAccess (Cluster&);
    
}

std::string TerminalUI::timezoneSelection
                                (const std::vector<std::string>& timezones) {

    return drawListMenu(MSG_TITLE_TIME_SETTINGS,
                        MSG_TIME_SETTINGS_TIMEZONE, timezones,
                        MSG_TIME_SETTINGS_TIMEZONE_HELP);
}

std::string TerminalUI::localeSelection
                                    (const std::vector<std::string>& locales) {

    return drawListMenu(MSG_TITLE_LOCALE_SETTINGS,
                        MSG_LOCALE_SETTINGS_LOCALE, locales,
                        MSG_LOCALE_SETTINGS_LOCALE_HELP);
}

std::vector<std::string> TerminalUI::networkHostnameSelection (
                                    const std::vector<std::string>& entries) {

    return drawFieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                         MSG_NETWORK_SETTINGS_HOSTID, entries,
                         MSG_NETWORK_SETTINGS_HOSTID_HELP);
}

/* TODO: Fix Internal/External interface mess, this method should be generic */
std::string TerminalUI::networkInterfaceSelection
                        (const std::vector<std::string>& interface) {

    return drawListMenu(MSG_TITLE_NETWORK_SETTINGS,
                        MSG_NETWORK_SETTINGS_EXTERNAL_IF, interface,
                        MSG_NETWORK_SETTINGS_EXTERNAL_IF_HELP);
}

//std::string drawNetworkExternalInterfaceSelection (Headnode&);
//std::string drawNetworkManagementInterfaceSelection (Headnode&);

std::vector<std::string> TerminalUI::networkAddress (
                                const std::vector<std::string>& addresses) {

    return drawFieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                         MSG_NETWORK_SETTINGS_INTERNAL_IPV4,
                         addresses,
                         MSG_NETWORK_SETTINGS_INTERNAL_IPV4_HELP);
}

