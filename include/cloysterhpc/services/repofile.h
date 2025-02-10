/*
 * Copyright 2025 Arthur Mendes <arthurmco@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOFILE_H_
#define CLOYSTERHPC_REPOFILE_H_

#include <concepts>
#include <expected>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

// #include <glibmm/fileutils.h>
// #include <glibmm/keyfile.h>

// #include <cloysterhpc/services/repo.h>

namespace cloysterhpc {
// @TODO: Move this to its own file
template <typename T>
concept NotCopyable = !std::is_copy_constructible_v<T> && !std::is_copy_assignable_v<T>;

template <typename T>
concept NotMoveable = !std::is_move_constructible_v<T> && !std::is_move_assignable_v<T>;

template <typename T>
concept NotCopiableMoveable = NotMoveable<T> && NotCopyable<T>;
/**
 * @brief Parser<P, T> means: P can parse and unparse Ts from streams. The
 * parsers are allowed to throw exceptions
 */
template<typename Parser_, typename T>
concept Parser =
    requires(Parser_ parser, std::istream& input, std::ostream& output, const T& parsed)
{
    { parser.parse(input) } -> std::same_as<T>;
    { parser.unparse(parsed, output) } -> std::same_as<void>;
};

// @TODO: Move this to its own file
/**
 * @brief ParserNoExc<P, T, E> means: P can parse and unparse Ts from streams. The
 * parsers are not allowed to throw exceptions, it must return errors of type E
 */
template<typename Parser_, typename T, typename E>
concept ParserNoExc =
    requires(Parser_ parser, std::istream& input, std::ostream& output, const T& parsed)
{
    { parser.parse(input) } noexcept -> std::same_as<std::expected<T, E>>;
    { parser.unparse(parsed, output) } noexcept -> std::same_as<std::expected<void, E>>;
};

// @TODO: Move this to its own file
/**
 * @brief Stored<T> means that T represends data in a disk that need to be
 * saved and restored after change.
 */
template<typename File_>
concept Saveable =
    requires(File_ file)
{
    { file.load() } -> std::same_as<void>;
    { file.save() } -> std::same_as<void>;
};

/**
 * @brief Represents a generic repository
 */
template<typename T>
concept Repository = requires(T repo, bool flag, const std::string& url) {
    { repo.name() } -> std::same_as<const std::string&>;
    { repo.enable(flag) } -> std::same_as<void>;
    { repo.enabled() } -> std::same_as<bool>;
    { repo.base_url(url) } -> std::same_as<void>;
    { repo.base_url() } -> std::same_as<std::optional<const std::string&>>;
};

class RPMRepo final {
    std::string m_group;
    std::string m_name;
    std::optional<std::string> m_base_url;
    std::optional<std::string> m_metalink;
    bool m_enabled = false;
    bool m_gpgcheck = false;
    std::string m_gpgkey;

public:
    [[nodiscard]] const std::string& name() const;
    void enable(bool flag);
    bool enabled();
    void base_url(std::string base_url);
    [[nodiscard]] std::optional<const std::string&> base_url() const;
};
//static_assert(Repository<RPMRepo>);

// @TODO: Move this to its own file
/**
 * @brief Parse/Unparse EL repositories
 */
class ELRepoParser final {
public:
    static std::vector<RPMRepo> parse(std::istream& input);
    static void unparse(
        const std::vector<RPMRepo>& repos, std::ostream& output);
};
static_assert(Parser<ELRepoParser, std::vector<RPMRepo>>);

// @TODO: Move this to its own file
/**
 * @brief Represents
 */
class RPMRepoFile final {
public:
    ~RPMRepoFile() = default;
    RPMRepoFile() = default;
    RPMRepoFile(const RPMRepoFile&) = delete;
    RPMRepoFile& operator=(const RPMRepoFile&) = delete;
    RPMRepoFile(RPMRepoFile&&) = delete;
    RPMRepoFile& operator=(RPMRepoFile&&) = delete;
    void save();
    void load();
};
static_assert(Saveable<RPMRepoFile>);
static_assert(NotCopiableMoveable<RPMRepoFile>);

// @TODO WIP, rename to RepoManager and replace the old RepoManager
/**
 * @brief Enable/disable, install/uninstall repositories to/from the filesystem
 */
template <typename Repo, typename RepoFile>
class NewRepoManager  final {
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


};

#endif
