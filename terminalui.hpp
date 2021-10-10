#ifndef _HPP_TERMINALUI_
#define _HPP_TERMINALUI_

#include <newt.h>

class TerminalUI {
private:
    bool hasEmptyField (const struct newtWinEntry *);
    void abortInstall (void);
    void beginInstall (void);
    void welcomeMessage (void);
#if 0 /* Port to C++ */
    void timeSettings (CLUSTER *);
    void localeSettings (CLUSTER *);
    void networkSettings (CLUSTER *);
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
    TerminalUI (void);
    ~TerminalUI (void);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* _HPP_TERMINALUI */
