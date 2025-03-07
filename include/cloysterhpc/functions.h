#ifndef CLOYSTERHPC_FUNCTIONS_H_
#define CLOYSTERHPC_FUNCTIONS_H_

#include "services/log.h"
#include <boost/process/child.hpp>
#include <boost/process/pipe.hpp>
#include <cloysterhpc/services/repos.h>
#include <filesystem>
#include <list>
#include <optional>
#include <string>

#include <boost/asio.hpp>
#include <cloysterhpc/services/runner.h>
#include <type_traits>

namespace cloyster {
// Globals
extern bool dryRun;

using OS = cloyster::models::OS;
std::shared_ptr<cloyster::services::BaseRunner> getRunner();
std::shared_ptr<cloyster::services::repos::RepoManager> getRepoManager(
    const OS& osinfo);

/**
 * A command proxy, to us to be able to get the
 * command output while the command is running
 *
 * We will use this in the progress dialog
 */
/**
 * @struct CommandProxy
 * @brief A command proxy to capture the command output while the command is
 * running.
 *
 * This structure is used to capture the output of a command in real-time,
 * useful for displaying progress in a dialog.
 */
struct CommandProxy {
    bool valid = false;
    boost::process::child child;
    boost::process::ipstream pipe_stream;

    /**
     * @brief Gets a line of output from the command.
     *
     * @return An optional string containing a line of output if available,
     * otherwise std::nullopt.
     */
    std::optional<std::string> getline();

    std::optional<std::string> getUntil(char c);
};

enum class Stream { Stdout, Stderr };

/* shell execution */

/**
 * @brief Executes a command and captures its output.
 *
 * @param command The command to execute.
 * @param output A list to store the output lines of the command.
 * @param overrideDryRun A flag to override the dryRun setting.
 * @return The exit code of the command.
 */
int runCommand(const std::string& command, std::list<std::string>& output,
    bool overrideDryRun = false);

/**
 * @brief Executes a command.
 *
 * @param command The command to execute.
 * @param overrideDryRun A flag to override the dryRun setting.
 * @return The exit code of the command.
 */
int runCommand(const std::string& command, bool overrideDryRun = false);

/**
 * @brief Executes a command and provides a proxy to capture its output
 * iteratively.
 *
 * @param command The command to execute.
 * @param overrideDryRun A flag to override the dryRun setting.
 * @return A CommandProxy to capture the command's output.
 */
CommandProxy runCommandIter(const std::string& command,
    Stream out = Stream::Stdout, bool overrideDryRun = false);

/* environment variables helper functions */

/**
 * @brief Retrieves the value of an environment variable.
 *
 * @param variable The name of the environment variable.
 * @return The value of the environment variable.
 */
std::string getEnvironmentVariable(const std::string&);

/* conf manipulation functions */

/**
 * @brief Reads a configuration value from a file.
 *
 * @param filename The name of the file to read from.
 * @return The configuration value as a string.
 */
std::string readConfig(const std::string&);

/**
 * @brief Writes a configuration value to a file.
 *
 * @param filename The name of the file to write to.
 */
void writeConfig(const std::string&);

/* helper function */

/**
 * @brief Creates an empty file, analogous to the `touch` command
 */
void touchFile(const std::filesystem::path& path);

/**
 * @brief Creates a directory at the specified path.
 *
 * @param path The path where the directory should be created.
 */
void createDirectory(const std::filesystem::path& path);

/**
 * @brief Removes a file.
 *
 * @param filename The name of the file to remove.
 */
void removeFile(std::string_view filename);

/**
 * @brief Creates a backup of a file.
 *
 * @param filename The name of the file to backup.
 */
void backupFile(std::string_view filename);

/**
 * @brief Changes a value in a configuration file.
 *
 * @param filename The name of the configuration file.
 * @param key The key of the value to change.
 * @param value The new value to set.
 */
void changeValueInConfigurationFile(
    const std::string&, const std::string&, std::string_view);

/**
 * @brief Adds a string to a file.
 *
 * @param filename The name of the file to add the string to.
 * @param string The string to add to the file.
 */
void addStringToFile(std::string_view filename, std::string_view string);
std::string getCurrentTimestamp();
std::string findAndReplace(const std::string_view& source,
    const std::string_view& find, const std::string_view& replace);

/**
 * @brief Copies a file, skip copying if it exists
 *
 * @param source The source file to copy.
 * @param destination The path where the source file will be copied.
 */
void copyFile(std::filesystem::path source, std::filesystem::path destination);

/**
 * @brief Create a file in path with the contents of data
 *
 * @param path The source file to install.
 * @param data The contents of the file to install
 */
void installFile(const std::filesystem::path& path, std::istream& data);

} // namespace cloyster

/**
 * @brief Generic functions. Be very judicious on what you put here. Is it
 * really generic?
 */
namespace cloyster::utils {

/**
 * @brief Returns true if [val] is in [container]
 */
inline bool isIn(const auto& container, const auto& val)
{
    return std::find(container.begin(), container.end(), val)
        == container.end();
}

/**
 * @brief Run [func] if cloyster::dryRun is false
 */
template <typename T>
    requires std::is_default_constructible_v<T>
inline T dryrun(const std::function<T()>& func, const std::string& msg)
{
    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: {}", msg);
        return T();
    }

    return func();
}

template <typename Path>
    requires std::is_convertible_v<Path, std::filesystem::path>
std::filesystem::directory_iterator openDir(const Path& path)
{
    return dryrun(
        static_cast<std::function<std::filesystem::directory_iterator()>>(
            [&path]() { return std::filesystem::directory_iterator(path); }),
        fmt::format("Dry Run: Would open directory {}", path.string()));
}

}

#endif // CLOYSTERHPC_FUNCTIONS_H_
