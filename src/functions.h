#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include <string> /* std::string */

#include "types.h"

namespace cloyster {
/* getopt */
int parseArguments(int, char**);

/* environment variables helper functions */
std::string getEnvironmentVariable(const std::string&);

/* conf manipulation functions */
std::string readConfig(const std::string&);
void writeConfig(const std::string&);

/* helper function */
void backupFile(const std::string_view&);
void changeValueInConfigurationFile(const std::string&, const std::string&,
                                    std::string_view);
void addStringToFile(std::string_view, std::string_view);

} /* namespace cloyster */

#endif /* FUNCTIONS_H */
