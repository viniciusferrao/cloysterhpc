#ifndef _HPP_TERMINALUI_
#define _HPP_TERMINALUI_

#include <newt.h>
#include "headnode.hpp"

class TerminalUI {
private:
    /* Ancillary private methods */
    //bool hasEmptyField (const struct newtWinEntry *);
    void abortInstall (void);
    void beginInstall (Headnode *);

    /* Interface helpers */
    std::string drawListMenu (const char *, const char *, const char * const *,
                              const char *);

    void helpMessage (const char*);
    void welcomeMessage (void);
    void timeSettings (Headnode *);
    void localeSettings (Headnode *);
#if 0 /* Port to C++ */
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
    TerminalUI (Headnode *);
    ~TerminalUI (void);

#ifdef _DEBUG_
    void debugInfo (void);
#endif /* _DEBUG_ */
};

#endif /* _HPP_TERMINALUI */
