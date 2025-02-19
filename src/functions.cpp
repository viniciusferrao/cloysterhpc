/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>

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

namespace cloyster {

using cloyster::services::BaseRunner;
using cloyster::services::DryRunner;
using cloyster::services::Runner;

namespace {
    std::tuple<bool, std::optional<std::string>> retrieveLine(
        boost::process::ipstream& pipe_stream,
        const std::function<std::string(boost::process::ipstream&)>& linecheck)
    {
        if (pipe_stream.good()) {
            return make_tuple(true, make_optional(linecheck(pipe_stream)));
        }

        return make_tuple(pipe_stream.good(), std::nullopt);
    }

    std::shared_ptr<BaseRunner> makeRunner(const bool dryRun)
    {
        if (dryRun) {
            return std::make_shared<DryRunner>();
        }

        return std::make_shared<Runner>();
    }

} // anonymous namespace

std::shared_ptr<BaseRunner> getRunner()
{
    static std::optional<std::shared_ptr<BaseRunner>> runner = std::nullopt;
    if (!runner) {
        runner = makeRunner(cloyster::dryRun);
    }

    return runner.value();
}

using cloyster::services::repos::RepoManager;

std::shared_ptr<RepoManager> getRepoManager(const OS& osinfo)
{
    static std::optional<std::shared_ptr<RepoManager>> repoManager
        = std::nullopt;
    if (!repoManager) {
        LOG_DEBUG("Initializing RepoManager");
        switch (osinfo.getPackageType()) {
            case OS::PackageType::RPM:
                repoManager = std::make_shared<RepoManager>(osinfo);
                break;
            case OS::PackageType::DEB:
                // @TODO Implement
                throw std::logic_error("Not implemented");
                break;
        }
    }

    return repoManager.value();
}

std::optional<std::string> CommandProxy::getline()
{

    if (!valid)
        return std::nullopt;

    auto [new_valid, out_line]
        = retrieveLine(pipe_stream, [this](boost::process::ipstream& pipe) {
              if (std::string line = ""; std::getline(pipe, line)) {
                  return line;
              }

              valid = false;
              return std::string {};
          });

    valid = new_valid;
    return out_line;
}

std::optional<std::string> CommandProxy::getUntil(char c)
{
    if (!valid)
        return std::nullopt;

    auto [new_valid, out_line]
        = retrieveLine(pipe_stream, [this, c](boost::process::ipstream& pipe) {
              if (std::string line = ""; std::getline(pipe, line, c)) {
                  return line;
              }

              valid = false;
              return std::string {};
          });

    valid = new_valid;
    return out_line;
}

CommandProxy runCommandIter(
    const std::string& command, Stream out, bool overrideDryRun)
{
    if (!cloyster::dryRun || overrideDryRun) {
        LOG_DEBUG("Running interative command: {}", command)
        boost::process::ipstream pipe_stream;

        if (out == Stream::Stderr) {
            boost::process::child child(
                command, boost::process::std_err > pipe_stream);
            return CommandProxy { .valid = true,
                .child = std::move(child),
                .pipe_stream = std::move(pipe_stream) };

        } else {
            boost::process::child child(
                command, boost::process::std_out > pipe_stream);
            return CommandProxy { .valid = true,
                .child = std::move(child),
                .pipe_stream = std::move(pipe_stream) };
        }
    }

    return CommandProxy {};
}

int runCommand(const std::string& command, std::list<std::string>& output,
    bool overrideDryRun)
{

    if (!cloyster::dryRun || overrideDryRun) {
        LOG_DEBUG("Running command: {}", command)
        boost::process::ipstream pipe_stream;
        boost::process::child child(
            command, boost::process::std_out > pipe_stream);

        std::string line;

        while (pipe_stream && std::getline(pipe_stream, line)) {
            LOG_TRACE("{}", line)
            output.emplace_back(line);
        }

        child.wait();
        LOG_DEBUG("Exit code: {}", child.exit_code())
        return child.exit_code();
    } else {
        LOG_WARN("Dry Run: {}", command)
        return 0;
    }
}

int runCommand(const std::string& command, bool overrideDryRun)
{
    std::list<std::string> discard;
    return runCommand(command, discard, overrideDryRun);
}

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
void writeConfig(const std::string& filename)
{
    boost::property_tree::ptree tree;

    tree.put("headnode.LANG", getEnvironmentVariable("LANG"));

    boost::property_tree::write_ini(filename, tree);
}

void touchFile(const std::filesystem::path& path)
{
    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would touch the file {}", path.string())
        return;
    }

    // BUG: I don't have to comment why this is a BUG, right?
    FILE* f = fopen(path.c_str(), "ab");
    (void)fflush(f);
    (void)fclose(f);
}

void createDirectory(const std::filesystem::path& path)
{
    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would create directory {}", path.string())
        return;
    }

    std::filesystem::create_directories(path);
    LOG_DEBUG("Created directory: {}", path.string())
}

/* Remove file */
void removeFile(std::string_view filename)
{
    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would remove file {}, if exists", filename)
        return;
    }

    LOG_DEBUG("Checking if file {} already exists on filesystem", filename)
    if (std::filesystem::exists(filename)) {
        LOG_DEBUG("Already exists")
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

    if (cloyster::dryRun) {
        LOG_WARN(
            "Dryn Run: Would create a backup copy of {} on {}", filename, backupFile);
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

    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would change the {} on {} in configuration file {}", value,
            key, filename);
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

    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would add a string in file {}:\n{}",
                 filename, string);
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
    if (cloyster::dryRun) {
        LOG_INFO(
            "Would copy file {} to {}", source.string(), destination.string())
        return;
    }

    try {
        std::filesystem::copy(source, destination);
    } catch (const std::filesystem::filesystem_error& ex) {
        if (ex.code().default_error_condition() == std::errc::file_exists) {
            LOG_WARN("File {} already exists, skip copying", source.string());
            return;
        }

        throw;
    }
}

}; // namespace cloyster
