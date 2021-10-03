/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef _H_TUI_
#define _H_TUI_

#include <newt.h>

#include "types.hpp"

/* Ancillary functions */
bool tuiHasEmptyField(const struct newtWinEntry *);
bool tuiIsValidIP(const char *);

/* Functions to generate the TUI */
void tuiStartup(void);
void tuiExit(void);
void tuiAbortInstall(void);
void tuiBeginInstall(CLUSTER *);
void tuiWelcomeMessage(void);
void tuiTimeSettings(CLUSTER *);
void tuiLocaleSettings(CLUSTER *);
void tuiNetworkSettings(CLUSTER *);
void tuiDirectoryServicesSettings(CLUSTER *);
void tuiNodeSettings(CLUSTER *);
void tuiInfinibandSettings(CLUSTER *);
/* Rearrange */
void tuiIPv4Settings(CLUSTER *);
/* End of Rearrange */
void tuiQueueSystemSettings(CLUSTER *);
void tuiSLURMSettings(CLUSTER *);
void tuiPBSSettings(CLUSTER *);
void tuiPostfixSettings(CLUSTER *);
void tuiPostfixRelaySettings(CLUSTER *);
void tuiPostfixSASLSettings(CLUSTER *);
void tuiUpdateSystem(CLUSTER *);
void tuiRemoteAccessSettings(CLUSTER *);

/* Debugging functions */
#ifdef _DEBUG_
void tuiDebugInfo(void);
#endif /* _DEBUG_ */

/* Deprecated functions */
#ifdef _DEPRECATED_
bool tuiCenteredMessage(void);
#endif

#endif /* _H_TUI_ */
