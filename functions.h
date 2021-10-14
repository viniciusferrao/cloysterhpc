#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string> /* std::string */

#include "types.h"

/* getopt */
int parseArguments(int, char **);

/* environment variables helper functions */
std::string getEnvironmentVariable (std::string const &);

/* conf manipulation functions */
std::string readConfig (const std::string &);
void writeConfig (const std::string &);

/* execution engine */
void runCommand (const std::string);

#endif /* FUNCTIONS_H */
