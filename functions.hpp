#ifndef _HPP_FUNCTIONS_
#define _HPP_FUNCTIONS_

#include <string> /* std::string */

#include "types.hpp"

/* getopt */
int parseArguments(int, char **);

/* environment variables helper functions */
std::string getEnvironmentVariable (std::string const &);

/* conf manipulation functions */
std::string readConfig (const std::string &);
void writeConfig (const std::string &);

#endif /* _HPP_FUNCTIONS_ */
