/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

/* This class is set to be reimplemented */
#ifndef CLOYSTERHPC_REPOS_H_
#define CLOYSTERHPC_REPOS_H_

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <cloysterhpc/concepts.h>
#include <cloysterhpc/models/os.h>
#include <unordered_map>
#include <unordered_set>

namespace cloyster {
extern std::string customRepofilePath;
};

namespace cloyster::services::repos {

/**
 * @brief Common interface for repositories.
 * @description It handles common ground between Debian and RPM repositories.
 */
class IRepository {
public:
    virtual ~IRepository() = default;

    [[nodiscard]] virtual std::string id() const
        = 0; // Unique identifier (e.g., uri for Debian, id for RPM)
    [[nodiscard]] virtual bool enabled() const
        = 0; // Whether the repository is enabled
    [[nodiscard]] virtual std::string name() const
        = 0; // Human-readable name or description
    [[nodiscard]] virtual std::optional<std::string> uri() const
        = 0; // URI or base URL of the repository
    [[nodiscard]] virtual std::filesystem::path source() const
        = 0; // File path where the repo is defined

    virtual void id(std::string value) = 0;
    virtual void enabled(bool value) = 0;
    virtual void name(std::string value) = 0;
    virtual void uri(std::optional<std::string> value) = 0;
    virtual void source(std::filesystem::path value) = 0;

    IRepository(const IRepository&) = default;
    IRepository(IRepository&&) = default;
    IRepository& operator=(const IRepository&) = default;
    IRepository& operator=(IRepository&&) = default;

protected:
    IRepository()
        = default; // Protected constructor to prevent direct instantiation
};

class RepoManager final {
    using OS = cloyster::models::OS;
    struct Impl;
    std::unique_ptr<Impl> m_impl;

public:
    ~RepoManager(); // required by Impl opaque type
    using Repositories
        = std::unordered_map<std::string, std::unique_ptr<IRepository>>;
    RepoManager();
    RepoManager(const RepoManager&) = delete;
    RepoManager(RepoManager&&) = delete;
    RepoManager& operator=(const RepoManager&) = delete;
    RepoManager& operator=(RepoManager&&) = delete;
    void initializeDefaultRepositories();
    void enable(const std::string& repo);
    void enable(const std::vector<std::string>& repos);
    void disable(const std::string& repo);
    void disable(const std::vector<std::string>& repos);
    void install(const std::filesystem::path& path);
    void install(const std::vector<std::filesystem::path>& paths);
    [[nodiscard]] std::vector<std::unique_ptr<const IRepository>>
        listRepos() const;
    [[nodiscard]] std::unique_ptr<const IRepository> repo(
        const std::string& repo) const;
};

};
#endif // CLOYSTERHPC_REPOS_H_
