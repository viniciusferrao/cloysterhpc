#ifndef _HPP_TERMINALUI_
#define _HPP_TERMINALUI_

#include <string>
#include <vector>

#include <newt.h>
#include "headnode.hpp"

class TerminalUI {
private:
    /* Ancillary private methods */
    void abortInstall (void);
    void beginInstall (Headnode *);
    bool hasEmptyField (const struct newtWinEntry *);

    /* Interface helpers */
    std::string drawListMenu (const char *, const char *, const char * const *,
                              const char *);
    std::vector<std::string> drawFieldMenu (const char *, const char *, 
                               struct newtWinEntry *, const char *);

    /* Interface screens */
    void drawHelpMessage (const char*);
    void drawWelcomeMessage (void);
    void drawTimeSettings (Headnode *);
    void drawLocaleSettings (Headnode *);
    void drawNetworkSettings (Headnode *);

#if 0 /* Port to C++ */
    void directoryServicesSettings (CLUSTER *);
    void nodeSettings (CLUSTER *);
    void infinibandSettings (CLUSTER *);
    /* Rearrange */
    void IPv4Settings (CLUSTER *);
    /* End of Rearrange */
    void queueSystemSettings (CLUSTER *);
    void SLURMSettings (CLUSTER *);
    void PBSSettings (CLUSTER *);
    void PostfixSettings (CLUSTER *);
    void PostfixRelaySettings (CLUSTER *);
    void PostfixSASLSettings (CLUSTER *);
    void updateSystem (CLUSTER *);
    void remoteAccessSettings (CLUSTER *);
#endif

public:
    TerminalUI (Headnode *);
    ~TerminalUI (void);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* _HPP_TERMINALUI */
