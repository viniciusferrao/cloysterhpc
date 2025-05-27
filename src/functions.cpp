/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/patterns/wrapper.h>

#include <chrono>
#include <cstdio> /* FILE*, fopen, fclose */
#include <cstdlib> /* getenv() */
#include <iostream>

#include <boost/process.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cloysterhpc/services/log.h>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fstream>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <tuple>

TEST_SUITE_BEGIN("cloyster");

namespace cloyster::functions {
using cloyster::services::repos::RepoManager;

/* Returns a specific environment variable when requested.
 * If the variable is not set it will return as an empty string. That's by
 * design and not considered a bug right now.
 */
std::string getEnvironmentVariable(const std::string& key)
{
    char* value = getenv(key.c_str());
    return value == nullptr ? std::string("") : std::string(value);
}

/* Read .conf file */
[[deprecated]]
std::string readConfig(const std::string& filename)
{
    boost::property_tree::ptree tree;

    try {
        boost::property_tree::ini_parser::read_ini(filename, tree);
    }

    catch (boost::property_tree::ini_parser_error& ex) {
        LOG_ERROR("Error: {}", ex.what())
    }

    std::string value = tree.get<std::string>("headnode.LANG", "en_US.utf8");

    LOG_TRACE("Read configFile variables:")
    LOG_TRACE("LANG: {}", value)

    return value;
}

/* Write .conf file function */
[[deprecated]]
void writeConfig(const std::string& filename)
{
    boost::property_tree::ptree tree;

    tree.put("headnode.LANG", getEnvironmentVariable("LANG"));

    boost::property_tree::write_ini(filename, tree);
}

void touchFile(const std::filesystem::path& path)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would touch the file {}", path.string())
        return;
    }

    if (cloyster::functions::exists(path)) {
        LOG_WARN("File already exists, skiping {}", path.string())
        return;
    }
    std::ofstream file(path, std::ios::app);
    if (!file) {
        throw std::runtime_error("Failed to touch file: " + path.string());
    }
}

void createDirectory(const std::filesystem::path& path)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would create directory {}", path.string())
        return;
    }

    std::filesystem::create_directories(path);
    LOG_DEBUG("Created directory: {}", path.string())
}

TEST_CASE("createDirectory - recursive creation and idempotency")
{
    using cloyster::services::Options;
    cloyster::Singleton<Options>::init(
        std::make_unique<Options>(Options{}));
    const std::filesystem::path testBaseDir = "test/output/functions";
    const std::filesystem::path targetDir = testBaseDir / "foo" / "bar" / "tar";

    // Clean up testBaseDir before the test to ensure a known state.
    if (std::filesystem::exists(testBaseDir)) {
        std::filesystem::remove_all(testBaseDir);
    }
    // After cleanup, testBaseDir should not exist.
    // createDirectory will create it and its children.
    CHECK_FALSE(std::filesystem::exists(testBaseDir));

    // --- Part 1: Test recursive creation ---
    cloyster::functions::createDirectory(targetDir);

    // Check if the target directory and its parents were created
    CHECK(std::filesystem::exists(targetDir));
    CHECK(std::filesystem::is_directory(targetDir));
    CHECK(std::filesystem::exists(testBaseDir / "foo" / "bar"));
    CHECK(std::filesystem::is_directory(testBaseDir / "foo" / "bar"));
    CHECK(std::filesystem::exists(testBaseDir / "foo"));
    CHECK(std::filesystem::is_directory(testBaseDir / "foo"));
    CHECK(std::filesystem::exists(testBaseDir));
    CHECK(std::filesystem::is_directory(testBaseDir));
}

/* Remove file */
void removeFile(std::string_view filename)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would remove file {}, if exists", filename)
        return;
    }

    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
        LOG_DEBUG("File {} deleted", filename)
    } else {
        LOG_DEBUG("File does not exist")
    }
}

/**
 * \brief Get the current timestamp as a string.
 *
 * This function generates the current timestamp in the format
 * "YYYYMMDD_HHMMSS".
 *
 * \return A string with the current timestamp.
 */
std::string getCurrentTimestamp()
{
    using clock = std::chrono::system_clock;
    using sec = std::chrono::seconds;

    std::chrono::time_point<clock> current_time = clock::now();
    auto result = fmt::format(
        "{:%FT%TZ}", std::chrono::time_point_cast<sec>(current_time));

    return result;
}

/* Backup file */
void backupFile(std::string_view filename)
{
    const auto& backupFile = fmt::format(
        "{}/backup{}_{}", installPath, filename, getCurrentTimestamp());

    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_WARN("Dryn Run: Would create a backup copy of {} on {}", filename,
            backupFile);
        return;
    }

    // Create the directory structure
    createDirectory(std::filesystem::absolute(backupFile).parent_path());

    std::fstream file(backupFile);
    if (!file.is_open()) {
        // Backup the file
        std::filesystem::copy_file(filename, backupFile);
        LOG_DEBUG("Created a backup copy of {} on {}", filename, backupFile)
    }
}

/* TODO:
 *  - Do not treat .conf files as .ini files!
 *  - Discover why boost removes comments on files while parsing INI config
 *  - Replace boost with glib's GKeyFile?
 *  - http://www.gtkbook.com/gtkbook/keyfile.html
 */
void changeValueInConfigurationFile(
    const std::string& filename, const std::string& key, std::string_view value)
{
    boost::property_tree::ptree tree;

    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would change the {} on {} in configuration file {}",
            value, key, filename);
        return;
    }

    try {
        boost::property_tree::ini_parser::read_ini(filename, tree);
    } catch (boost::property_tree::ini_parser_error& ex) {
        LOG_ERROR("Error: {}", ex.what())
    }

    tree.put(key, value);
    boost::property_tree::write_ini(filename, tree);
}

void addStringToFile(std::string_view filename, std::string_view string)
{
    // Check if file already contains the string to avoid duplicate lines.
    std::ifstream ifs(std::string { filename });
    std::string content((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));

    if (content.find(string) != std::string::npos) {
        LOG_DEBUG("File {} already contains line(s):\n{}\n", filename, string)
        return;
    }

#ifdef _LIBCPP_VERSION
    std::ofstream file(filename, std::ios_base::app);
#else
    std::ofstream file(std::string { filename }, std::ios_base::app);
#endif
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_WARN(
            "Dry Run: Would add a string in file {}:\n{}", filename, string);
        return;
    }

    if (!file.is_open())
        throw std::runtime_error(
            fmt::format("Error opening file: {}", filename));

    file << string;
    LOG_DEBUG("Added line(s):\n{}\n => to file: {}", string, filename)
}

std::string findAndReplace(const std::string_view& source,
    const std::string_view& find, const std::string_view& replace)
{
    std::string result { source };
    std::string::size_type pos = 0;

    while ((pos = result.find(find, pos)) != std::string::npos) {
        result.replace(pos, find.length(), replace);
        pos += replace.length();
    }

    return result;
}

/// Copy a file, ignore if file exists
void copyFile(std::filesystem::path source, std::filesystem::path destination)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO(
            "Would copy file {} to {}", source.string(), destination.string())
        return;
    }

    try {
        LOG_DEBUG("Copying file {} to {}", source, destination);
        std::filesystem::copy(source, destination);
    } catch (const std::filesystem::filesystem_error& ex) {
        if (ex.code().default_error_condition() == std::errc::file_exists) {
            LOG_WARN("File {} already exists, skip copying", source);
            return;
        }

        throw;
    }
}

bool exists(const std::filesystem::path& path)
{
    return std::filesystem::exists(path);
}

void installFile(const std::filesystem::path& path, std::istream& data)
{
    auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would install file {}", path.string());
        return;
    }

    if (cloyster::functions::exists(path)) {
        LOG_WARN("File already exists: {}, skipping", path.string());
        return;
    }

    std::ofstream fil(path);
    fil << data.rdbuf();
}

void installFile(const std::filesystem::path& path, std::string&& data)
{
    std::istringstream stringData(std::move(data));
    installFile(path, stringData);
}

HTTPRepo createHTTPRepo(const std::string_view repoName)
{
    const auto confPath = fmt::format("/etc/httpd/conf.d/{}.conf", repoName);
    const auto repoFolder = fmt::format("/var/www/html/repos/{}", repoName);
    // @FIXME:  Use the HN hostname instead of localhost to make it work in the
    // nodes
    HTTPRepo repo(repoFolder, std::string(repoName),
        fmt::format("http://localhost/repos/{}", repoName));
    if (cloyster::functions::exists(confPath)) {
        LOG_WARN("Skipping the creation of HTTP repository, {} already exists",
            confPath);
        return repo;
    }

    cloyster::functions::createDirectory("/var/www/html/repos/");
    LOG_INFO("Creating HTTP repository {} at {}", confPath, repoFolder);
    auto runner = cloyster::Singleton<IRunner>::get();
    cloyster::functions::createDirectory(repoFolder);
    cloyster::functions::installFile(confPath,
        fmt::format(
            R"(<Directory "{0}">
Options +Indexes +FollowSymLinks
AllowOverride None
Require all granted
IndexOptions FancyIndexing VersionSort NameWidth=* HTMLTable Charset=UTF-8
</Directory>
)",
            repoFolder));

    runner->checkCommand("apachectl configtest");
    runner->checkCommand("systemctl restart httpd");
    return repo;
}

void backupFilesByExtension(
    const wrappers::DestinationPath& backupPath,
    const wrappers::SourcePath& sourcePath,
    const wrappers::Extension& extension)
{
    const auto opts = cloyster::Singleton<services::Options>::get();
    if (opts->shouldForce("backups")) {
        std::filesystem::remove_all(backupPath);
    }

    if (!cloyster::functions::exists(backupPath)) {
        LOG_INFO("Backing up {} files from {} to {}", extension, sourcePath, backupPath);
        cloyster::functions::createDirectory(backupPath.get());
        cloyster::functions::moveFilesWithExtension(sourcePath.get(), backupPath.get(), extension.get());
    } else {
        LOG_INFO("Backup path {} already exists, skipping", backupPath);
    }
}
TEST_SUITE_END();

}; // namespace cloyster
