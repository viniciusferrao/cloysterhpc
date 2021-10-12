#ifndef _HPP_TERMINALUI_
#define _HPP_TERMINALUI_

#include <string>
#include <vector>

#include <newt.h>
#include "cluster.hpp"
#include "headnode.hpp"

class TerminalUI {
private:
    /* Ancillary private methods */
    void abortInstall (void);
    void beginInstall (Cluster *, Headnode *);
    bool hasEmptyField (const struct newtWinEntry *);
#ifdef _DEBUG_
    void drawDebugEntries(const struct newtWinEntry *);
#endif

    /* Interface helpers */
    bool drawYesNoQuestion (const char *, const char *, const char *);
    std::string drawListMenu (const char *, const char *, const char * const *,
                              const char *);
    std::vector<std::string> drawFieldMenu (const char *, const char *, 
                               struct newtWinEntry *, const char *);
    std::vector<std::string> drawIPSettings (const char *, const char *, 
                                             const char */*, Network **/);

    /* Interface screens */
    void drawHelpMessage (const char *);
    void drawWelcomeMessage (void);
    void drawTimeSettings (Headnode *);
    void drawLocaleSettings (Headnode *);
    void drawNetworkSettings (Cluster *, Headnode *);
        void drawNetworkHostnameSettings (Headnode *);
        void drawNetworkExternalInterfaceSelection (Headnode *);
        void drawNetworkManagementInterfaceSelection (Headnode *);
        void drawNetworkManagementAddress (Headnode *);
        void drawNetworkManagementXCATRange (Cluster *);
    void drawInfinibandSettings(Cluster *);
    void drawDirectoryServicesSettings (Cluster *);
        void drawDirectoryServicesPassword (Cluster *);
        void drawDirectoryServicesDisableDNSSEC (Cluster *);
    void drawNodeSettings (Cluster *);
    void drawQueueSystemSettings (Cluster *);
        void drawSLURMSettings (Cluster *);
        void drawPBSSettings (Cluster *);
    void drawPostfixSettings (Cluster *);
        void drawPostfixEnable (Cluster *);
        void drawPostfixProfile (Cluster *);
        void drawPostfixRelaySettings (Cluster *);
        void drawPostfixSASLSettings (Cluster *);
    void drawUpdateSystem (Cluster *);
    void drawRemoteAccess (Cluster *);

public:
    TerminalUI (Cluster *, Headnode *);
    ~TerminalUI (void);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* _HPP_TERMINALUI */
