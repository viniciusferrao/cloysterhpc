/*
 * Copyright 2025 Arthur Mendes <arthurmco@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOFILE_H_
#define CLOYSTERHPC_REPOFILE_H_

#include <concepts>
#include <expected>
#include <filesystem>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include <cloysterhpc/repos.h> // @TODO: Remove this header
#include <cloysterhpc/concepts.h>

namespace cloyster {
/**
 * @brief Represents a generic repository
 * @TODO MERGE THIS WITH cloyterhpc/repos.h
 */

/*
 * @TODO MERGE THIS WITH cloyterhpc/repos.h
 */
struct RPMRepository final {
    std::string id;
    bool enabled = true;
    std::string name;
    std::optional<std::string> baseurl;
    std::optional<std::string> metalink;
    bool gpgcheck = true;
    std::string gpgkey;
    std::filesystem::path source;
    std::string group;
};
// static_assert(Repository<RPMRepository>);
static_assert(IsRepository<RPMRepository>);

// @TODO: Move this to its own file
/**
 * @brief Parse/Unparse RPM repositories
 */
class RPMRepositoryParser final {
public:
    static void parse(std::istream& input, std::vector<RPMRepository>& output);
    static void unparse(
        const std::vector<RPMRepository>& repos, std::ostream& output);
};
static_assert(cloyster::concepts::Parser<RPMRepositoryParser, std::vector<RPMRepository>>);

// @TODO: Move this to its own file
/**
 * @brief Represents
 */
class RPMRepositoryFile final {
public:
    ~RPMRepositoryFile() = default;
    RPMRepositoryFile() = default;
    RPMRepositoryFile(const RPMRepositoryFile&) = delete;
    RPMRepositoryFile& operator=(const RPMRepositoryFile&) = delete;
    RPMRepositoryFile(RPMRepositoryFile&&) = delete;
    RPMRepositoryFile& operator=(RPMRepositoryFile&&) = delete;
    void save();
    void load();
};
static_assert(cloyster::concepts::IsSaveable<RPMRepositoryFile>);
static_assert(cloyster::concepts::NotCopiableMoveable<RPMRepositoryFile>);

// @TODO WIP, rename to RepoManager and replace the old RepoManager
/**
 * @brief Enable/disable, install/uninstall repositories to/from the filesystem
 */
template <typename Repo, typename RepoFile> class NewRepoManager final {
public:
    ~NewRepoManager() = default;
    NewRepoManager() = default;
    NewRepoManager(const NewRepoManager&) = delete;
    NewRepoManager& operator=(const NewRepoManager&) = delete;
    NewRepoManager(NewRepoManager&&) = delete;
    NewRepoManager& operator=(NewRepoManager&&) = delete;

    void load(const std::filesystem::path& path);
    void enable(const std::string& id);
    void enable(std::vector<std::string> ids);
    void disable(const std::string& id);
    void disable(std::vector<std::string> ids);
    const std::vector<Repo>& listRepos() const;
};

} // namespace cloyster;

#endif
