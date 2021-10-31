#ifndef TERMINALUI_H
#define TERMINALUI_H

#include <string>
#include <vector>

#include <newt.h>
#include "cluster.h"
#include "headnode.h"

class TerminalUI {
private:
    /* Ancillary private methods */
    void abortInstall ();
    void beginInstall (Cluster&, Headnode&);
    bool hasEmptyField (const struct newtWinEntry *);
#ifdef _DEBUG_
    void drawDebugEntries(const struct newtWinEntry *);
#endif

    /* Interface helpers */
    bool drawYesNoQuestion (const char *, const char *, const char *);
    std::string drawListMenu (const char *, const char *,
                              const std::vector<std::string>&, const char *);
    std::vector<std::string> drawFieldMenu (const char *, const char *,
                                            const std::vector<std::string>&,
                                            const char *);
    std::vector<std::string> drawIPSettings (const char *, const char *, 
                                             const char */*, Network **/);

    /* Interface screens */
    void drawHelpMessage (const char *);
    void drawWelcomeMessage ();
    void drawTimeSettings (Cluster&);
    void drawLocaleSettings (Cluster&);
    void drawNetworkSettings (Cluster&, Headnode&);
        void drawNetworkHostnameSettings (Cluster&);
        void drawNetworkExternalInterfaceSelection (Headnode&);
        void drawNetworkManagementInterfaceSelection (Headnode&);
        void drawNetworkManagementAddress (Headnode&);
        void drawNetworkManagementXCATRange (Cluster&);
    void drawInfinibandSettings(Cluster&);
    void drawDirectoryServicesSettings (Cluster&);
        void drawDirectoryServicesPassword (Cluster&);
        void drawDirectoryServicesDisableDNSSEC (Cluster&);
    void drawNodeSettings (Cluster&);
    void drawQueueSystemSettings (Cluster&);
        void drawSLURMSettings (Cluster&);
        void drawPBSSettings (Cluster&);
    void drawPostfixSettings (Cluster&);
        void drawPostfixEnable (Cluster&);
        void drawPostfixProfile (Cluster&);
        void drawPostfixRelaySettings (Cluster&);
        void drawPostfixSASLSettings (Cluster&);
    void drawUpdateSystem (Cluster&);
    void drawRemoteAccess (Cluster&);

public:
    TerminalUI (Cluster&, Headnode&);
    ~TerminalUI ();

    /* Refactoring to MVC */
    TerminalUI ();
    TerminalUI (Cluster&);
    std::string timezoneSelection (const std::vector<std::string>&);
    std::string localeSelection (const std::vector<std::string>&);
    std::vector<std::string> networkHostnameSelection
                                              (const std::vector<std::string>&);
    std::string networkInterfaceSelection (const std::vector<std::string>&);
    std::vector<std::string> networkAddress (const std::vector<std::string>&);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* TERMINALUI_H */
