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

#include <cloysterhpc/concepts.h>

namespace cloyster::services::repos {

template <typename T>
concept IsRepository = requires(T repo, const T& crepo,
                                const std::string& str, bool bol,
                                const std::optional<std::string>& optstr)
{
    { repo.id() } -> std::convertible_to<std::string>;
    { repo.enabled() } -> std::convertible_to<bool>;
    { repo.name() } -> std::convertible_to<std::string>;
    { repo.baseurl() } -> std::convertible_to<std::optional<std::string>>;
    { repo.metalink() } -> std::convertible_to<std::optional<std::string>>;
    { repo.gpgcheck() } -> std::convertible_to<bool>;
    { repo.gpgkey() } -> std::convertible_to<std::string>;
    { repo.source() } -> std::convertible_to<std::filesystem::path>;

    repo.id(str);
    repo.enabled(bol);
    repo.name(str);
    repo.baseurl(optstr);
    repo.metalink(str);
    repo.gpgcheck(bol);
    repo.gpgkey(str);
    repo.source(str);
};

/**
 * @brief Base class for repository
 *
 * @TODO: This is a shameless copy of RPM repository.
 *   It will require generalization to accomodate debian
 *   like repositories in future
 */
class BaseRepository {
public:
    BaseRepository() = default;
    BaseRepository(const BaseRepository&) = delete;
    BaseRepository(BaseRepository&&) = default;
    BaseRepository& operator=(const BaseRepository&) = delete;
    BaseRepository& operator=(BaseRepository&&) = default;
    virtual ~BaseRepository() = default;
    [[nodiscard]] virtual std::string id() const = 0;
    [[nodiscard]] virtual bool enabled() const = 0;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string group() const = 0;
    [[nodiscard]] virtual std::optional<std::string> baseurl() const = 0;
    [[nodiscard]] virtual std::optional<std::string> metalink() const = 0;
    [[nodiscard]] virtual bool gpgcheck() const = 0;
    [[nodiscard]] virtual std::string gpgkey() const = 0;
    [[nodiscard]] virtual std::filesystem::path source() const = 0;

    virtual void id(std::string) = 0;
    virtual void enabled(bool) = 0;
    virtual void name(std::string) = 0;
    virtual void group(std::string) = 0;
    virtual void baseurl(std::optional<std::string>) = 0;
    virtual void metalink(std::optional<std::string>) = 0;
    virtual void gpgcheck(bool) = 0;
    virtual void gpgkey(std::string) = 0;
    virtual void source(std::filesystem::path) = 0;
};
static_assert(IsRepository<BaseRepository>);


class RPMRepository final : public BaseRepository {
    std::string m_id;
    bool m_enabled = true;
    std::string m_name;
    std::optional<std::string> m_baseurl;
    std::optional<std::string> m_metalink;
    bool m_gpgcheck = true;
    std::string m_gpgkey;
    std::filesystem::path m_source;
    std::string m_group;
public:
    RPMRepository() = default;
    ~RPMRepository() override = default;
    RPMRepository(const RPMRepository&) = delete;
    RPMRepository(RPMRepository&&) = default;
    RPMRepository& operator=(const RPMRepository&) = delete;
    RPMRepository& operator=(RPMRepository&&) = default;

    [[nodiscard]] std::string id() const override { return m_id; };
    [[nodiscard]] bool enabled() const override { return m_enabled; };
    [[nodiscard]] std::string name() const override { return m_name; };
    [[nodiscard]] std::string group() const override { return m_group; };
    [[nodiscard]] std::optional<std::string> baseurl() const override { return m_baseurl; };
    [[nodiscard]] std::optional<std::string> metalink() const override { return m_metalink; };
    [[nodiscard]] bool gpgcheck() const override { return m_gpgcheck; };
    [[nodiscard]] std::string gpgkey() const override { return m_gpgkey; };
    [[nodiscard]] std::filesystem::path source() const override { return m_source; };

    void id(std::string value) override { m_id = value; };
    void enabled(bool enabled) override { m_enabled = enabled; };
    void name(std::string name) override { m_name = name; };
    void group(std::string group) override { m_group = group; };
    void baseurl(std::optional<std::string> baseurl) override { m_baseurl = baseurl; };
    void metalink(std::optional<std::string> metalink) override { m_metalink = metalink; };
    void gpgcheck(bool gpgcheck) override { m_gpgcheck = gpgcheck; };
    void gpgkey(std::string gpgkey) override { m_gpgkey = gpgkey; };
    void source(std::filesystem::path source) override { m_source = source; };
};
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
static_assert(cloyster::concepts::IsParser<RPMRepositoryParser, std::vector<RPMRepository>>);

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

} // namespace cloyster;

#endif
