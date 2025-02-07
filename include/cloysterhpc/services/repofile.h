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
/**
 * @brief Base class for marking a superclass as non-copiable and non-moveable
 *
 * We can recover copiability and moveability by explicitly using smart pointers
 */
class NonCopiableNonMoveable {
public:
    NonCopiableNonMoveable(const NonCopiableNonMoveable&) = delete;
    NonCopiableNonMoveable& operator=(const NonCopiableNonMoveable&) = delete;
    NonCopiableNonMoveable(NonCopiableNonMoveable&&) = delete;
    NonCopiableNonMoveable& operator=(NonCopiableNonMoveable&&) = delete;
    NonCopiableNonMoveable() = default;
    ~NonCopiableNonMoveable() = delete;
};

// @TODO: Move this to its own file
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
 * @brief Parser<P, T, E> means: P can parse and unparse Ts from streams. The
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
class Repository {
public:
    virtual ~Repository() = default;

    virtual const std::string& name() const = 0;
    virtual void enable(bool flag) = 0;
    virtual const bool enabled() = 0;
    virtual void base_url(std::string base_url) = 0;
    virtual const std::optional<const std::string&> base_url() const = 0;
};

class RPMRepo final : public Repository {
    std::string m_group;
    std::string m_name;
    std::optional<std::string> m_base_url;
    std::optional<std::string> m_metalink;
    bool m_enabled = false;
    bool m_gpgcheck = false;
    std::string m_gpgkey;

public:
    [[nodiscard]] const std::string& name() const override;
    void enable(bool flag) override;
    const bool enabled() override;
    void base_url(std::string base_url) override;
    const std::optional<const std::string&> base_url() const override;
};

// @TODO: Move this to its own file
/**
 * @brief Parse/Unparse EL repositories
 */
class ELRepoParser final {
    static std::vector<RPMRepo> parse(std::istream input);
    static void unparse(
        const std::vector<RPMRepo>& repos, std::ostream output);
};
static_assert(Parser<ELRepoParser, std::vector<RPMRepo>>);

// @TODO: Move this to its own file
/**
 * @brief Represents
 */
class ELRepoFile final : public NonCopiableNonMoveable {
public:
    void save();
    void load();
};
static_assert(Saveable<ELRepoFile>);

// @TODO WIP, rename to RepoManager and replace the old RepoManager
template<typename Repo, typename RepoFile>
class NewRepoManager final {
public:
    void load(const std::filesystem::path& file);
    void enable(const std::string& id);
    void enable(std::vector<std::string> ids);
    void disable(const std::string& id);
    void disable(std::vector<std::string> ids);
    const std::vector<Repo>& listRepos() const;
};

}

#endif
