#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include <string>
#include <optional>
#include <vector>

namespace cloyster {
/* getopt */
int parseArguments(int, char**);

/* shell execution */
int runCommand(const std::string& command,
               const std::optional<std::vector<std::string>>& output);
int runCommand(const std::string& command);

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
