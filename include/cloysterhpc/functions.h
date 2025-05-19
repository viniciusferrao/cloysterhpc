#ifndef CLOYSTERHPC_FUNCTIONS_H_
#define CLOYSTERHPC_FUNCTIONS_H_

#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/patterns/singleton.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/utils/enums.h>
#include <cloysterhpc/utils/string.h>
#include <algorithm>
#include <filesystem>
#include <string>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <boost/asio.hpp>
#include <cloysterhpc/services/runner.h>
#include <type_traits>

namespace cloyster {

// Globals, intialized by the command line parser
template <typename B, typename T, typename... Args>
constexpr std::unique_ptr<B> makeUniqueDerived(Args... args)
{
    return static_cast<std::unique_ptr<B>>(std::make_unique<T>(args...));
}

using cloyster::models::OS;
using cloyster::services::IRunner;

/* shell execution */

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
void installFile(const std::filesystem::path& path, std::string&& data);

bool exists(const std::filesystem::path& path);

struct HTTPRepo {
    std::filesystem::path directory;
    std::string name;
    std::string url;
};

HTTPRepo createHTTPRepo(const std::string_view repoName);


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
        != container.end();
}

TEST_SUITE_BEGIN("cloyster::utils");

TEST_CASE("isIn") {
    const auto container = {1,2,3};
    CHECK(isIn(container, 3) == true);
    CHECK(isIn(container, 4) == false);
};

/**
 * @brief Run [func] if cloyster::dryRun is false
 */
template <typename T>
    requires std::is_default_constructible_v<T>
inline T dryrun(const std::function<T()>& func, const std::string& msg)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: {}", msg);
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

std::vector<std::string> getFilesByExtension(
    const auto& path,
    const auto& extension
) {
    std::vector<std::string> result;
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() &&
            entry.path().extension() == extension) {
            result.push_back(entry.path().filename().string());
        }
    }

    return result;
}

TEST_CASE("getFilesByExtension") {
    const auto files = getFilesByExtension("repos/", ".conf");
    CHECK(files.size() == 1);
    CHECK(files[0] == "repos.conf");
}

void removeFilesWithExtension(
    const auto& path,
    const auto& extension
) {
    namespace fs = std::filesystem;

    std::string extensionLower = string::lower(std::string(extension));

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string ext = string::lower(entry.path().extension().string());

            if (ext == extensionLower) {
                fs::remove(entry.path());
            }
        }
    }
}

TEST_CASE("removeFilesWithExtension") {
    const std::filesystem::path path = "test/output/utils";
    touchFile(path / "test.txt");
    CHECK(getFilesByExtension(path, ".txt").size() == 1);
    removeFilesWithExtension(path, ".txt");
    CHECK(getFilesByExtension(path, ".txt").size() == 0);
}


// @FIXME: Yes, curl works I know, but no.. fix this, use boost for
//   requests and asynchronous I/O so we can run a bunch of these
//   concurrently
std::string getHttpStatus(const auto& url, const std::size_t maxRetries = 3)
{
    auto runner = cloyster::Singleton<IRunner>::get();
    auto opts = cloyster::Singleton<services::Options>::get();
    if (opts->shouldSkip("http-status")) {
        LOG_WARN("Skipping HTTP status check for {}, assuming 200 (reason: --skip=http-status in the command line)", url);
        return "200";
    }
    constexpr auto getHttpStatusInner = [](const auto& url, const auto& runner) {
        auto lines = runner->checkOutput(fmt::format(R"(bash -c "curl -sSLI {} | awk '/HTTP/ {{print $2}}' | tail -1" )", url));
        if (lines.size() > 0) {
            return lines[0];
        }
        return std::string("CURL ERROR");
    };


    std::string header;
    for (std::size_t i = 0; i < maxRetries; ++i) {
        header = getHttpStatusInner(url, runner);     
        LOG_DEBUG("HTTP status of {}: {}", url, header);
        if (header == "200") {
            return header;
        } else if (header.starts_with("5")) {
            LOG_DEBUG("HTTP {} error, retry {}", header, i);
            continue;
        }
    }
    return header;
};

[[noreturn]] void abort(const fmt::string_view& fmt, auto&&... args) {
    throw std::runtime_error(fmt::format(fmt::runtime(fmt), std::forward<decltype(args)>(args)...));
}


TEST_SUITE_END();

} // namespace cloyster::utils

#endif // CLOYSTERHPC_FUNCTIONS_H_
