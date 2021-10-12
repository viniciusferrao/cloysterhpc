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

    /* Interface helpers */
    bool drawYesNoQuestion (const char *, const char *, const char *);
    std::string drawListMenu (const char *, const char *, const char * const *,
                              const char *);
    std::vector<std::string> drawFieldMenu (const char *, const char *, 
                               struct newtWinEntry *, const char *);

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
    void drawDirectoryServicesSettings (Cluster *);
        void drawDirectoryServicesPassword (Cluster *);
        void drawDirectoryServicesDisableDNSSEC (Cluster *);
    void drawNodeSettings (Cluster *);
    
    void drawQueueSystemSelection (Cluster *);
        void drawSLURMSettings (Cluster *);
        void drawPBSSettings (Cluster *);


#if 0 /* Port to C++ */
    void infinibandSettings (CLUSTER *);
    /* Rearrange */
    void IPv4Settings (CLUSTER *);
    /* End of Rearrange */
    void PostfixSettings (CLUSTER *);
    void PostfixRelaySettings (CLUSTER *);
    void PostfixSASLSettings (CLUSTER *);
    void updateSystem (CLUSTER *);
    void remoteAccessSettings (CLUSTER *);
#endif

public:
    TerminalUI (Cluster *, Headnode *);
    ~TerminalUI (void);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* _HPP_TERMINALUI */
