/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <magic_enum/magic_enum.hpp>
#include <utility>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>

/**
 * @TODO: (in this file)
 *
 * - Add a repository file class to handle the save/loading, as
 *   it is too ad-hoc now.
 * - Maybe break this files into smaller files, but DO NOT,
 *   expose implementation details in "global" headers.
 */

using cloyster::OS;
using cloyster::services::files::IsKeyFileReadable;
using cloyster::services::files::KeyFile;
using cloyster::services::repos::IRepository;

namespace cloyster::services::repos {

class DebianRepository : public IRepository {
private:
    std::string m_type; // "deb" or "deb-src"
    std::optional<std::string> m_options; // Optional, e.g., "[arch=amd64]"
    std::string m_uri; // URI, e.g., "http://deb.debian.org/debian"
    std::string m_distribution; // Distribution, e.g., "bookworm"
    std::vector<std::string>
        m_components; // Components, e.g., "main", "contrib", "non-free"
    std::filesystem::path m_source; // File where this is saved

public:
    explicit DebianRepository(const std::string& line);

    [[nodiscard]] std::string type() const;
    [[nodiscard]] std::optional<std::string> options() const;
    [[nodiscard]] std::optional<std::string> uri() const override;
    [[nodiscard]] std::string distribution() const;
    [[nodiscard]] std::vector<std::string> components() const;
    [[nodiscard]] std::filesystem::path source() const override;

    void type(std::string value);
    void options(std::optional<std::string> value);
    void uri(std::optional<std::string> value) override;
    void distribution(std::string value);
    void components(std::vector<std::string> value);
    void source(const std::filesystem::path value) override;
};

class RPMRepository final : public IRepository {
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
    [[nodiscard]] std::optional<std::string> uri() const override
    {
        return baseurl();
    };
    [[nodiscard]] std::filesystem::path source() const override
    {
        return m_source;
    };
    [[nodiscard]] std::string group() const { return m_group; };
    [[nodiscard]] std::optional<std::string> baseurl() const
    {
        return m_baseurl;
    };
    [[nodiscard]] std::optional<std::string> metalink() const
    {
        return m_metalink;
    };
    [[nodiscard]] bool gpgcheck() const { return m_gpgcheck; };
    [[nodiscard]] std::string gpgkey() const { return m_gpgkey; };

    void id(std::string value) override { m_id = value; };
    void enabled(bool enabled) override { m_enabled = enabled; };
    void name(std::string name) override { m_name = name; };
    void uri(std::optional<std::string> value) override { baseurl(value); };
    void source(std::filesystem::path source) override { m_source = source; };
    void group(std::string group) { m_group = std::move(group); };
    void baseurl(std::optional<std::string> baseurl)
    {
        m_baseurl = std::move(baseurl);
    };
    void metalink(std::optional<std::string> metalink)
    {
        m_metalink = std::move(metalink);
    };
    void gpgcheck(bool gpgcheck) { m_gpgcheck = gpgcheck; };
    void gpgkey(std::string gpgkey) { m_gpgkey = std::move(gpgkey); };

    void valid() const;
};

class RPMRepositoryParser final {
public:
    static void parse(
        const std::filesystem::path& path, std::vector<RPMRepository>& output);
    static void unparse(const std::vector<RPMRepository>& repos,
        const std::filesystem::path& output);
};
// static_assert(cloyster::concepts::IsParser<RPMRepositoryParser,
// std::vector<RPMRepository>>);

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
static_assert(cloyster::concepts::NotCopiableNotMoveable<RPMRepositoryFile>);

}; // namespace cloyster::services::repos {

namespace {

using cloyster::services::repos::RPMRepository;
using cloyster::services::repos::RPMRepositoryParser;

constexpr std::string_view CLOYSTER_REPO_EL8 = {
#include "cloysterhpc/repos/el8/cloyster.repo"

};

constexpr std::string_view CLOYSTER_REPO_EL9 = {
#include "cloysterhpc/repos/el9/cloyster.repo"
};

constexpr std::string_view CLOYSTER_REPO_EL10 = {
#include "cloysterhpc/repos/el10/cloyster.repo"
};

// @TODO Make this a RPMRepositoryFile method
void loadRPMRepos(
    const std::filesystem::path& source, auto& m_repos, auto& m_filesLoaded)
{
    if (m_filesLoaded.contains(source)) {
        LOG_WARN("Skipping the loading of repository file {}, because it was "
                 "already loaded before",
            source.string());
        return;
    }
    m_filesLoaded.insert(source);

    LOG_DEBUG("Loading RPM Repositories from file {}", source.string());
    constexpr auto parser = RPMRepositoryParser();
    std::vector<RPMRepository> output;
    parser.parse(source, output);
    for (auto&& repo : output) {
        LOG_ASSERT(repo.id().size() > 0, "BUG In the RPM Parser");
        LOG_DEBUG("Loaded [{}] {}", source.string(), repo.id());
        m_repos.insert(
            { repo.id(), std::make_shared<RPMRepository>(std::move(repo)) });
    }
}

void installFile(const std::filesystem::path& path, std::istream& data)
{
    if (cloyster::dryRun) {
        LOG_WARN("Dry Run: Would install file {}", path.string());
        return;
    }

    if (std::filesystem::exists(path)) {
        LOG_WARN("File already exists: {}, skipping", path.string());
        return;
    }

    std::ofstream fil(path);
    fil << data.rdbuf();
}

constexpr auto getDefaultPath(const auto& osinfo)
{
    switch (osinfo.getPlatform()) {
        case OS::Platform::el8:
        case OS::Platform::el9:
        case OS::Platform::el10:
            return "/etc/yum.repos.d/cloyster.repo";
        default:
            throw std::logic_error(
                "Not implemented: Debian default repository path");
    }
}

auto loadDefaults(const auto& m_os, auto& m_repos, auto& m_filesLoaded)
{
    auto path = getDefaultPath(m_os);
    std::istringstream stream;
    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL8, cloyster::productName));
            installFile(path, stream);
            loadRPMRepos(path, m_repos, m_filesLoaded);
            break;
        case OS::Platform::el9:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL9, cloyster::productName));
            installFile(path, stream);
            loadRPMRepos(path, m_repos, m_filesLoaded);
            break;
        case OS::Platform::el10:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL10, cloyster::productName));
            installFile(path, stream);
            loadRPMRepos(path, m_repos, m_filesLoaded);
            break;
        default:
            throw std::runtime_error(fmt::format("Unsupported platform {}",
                magic_enum::enum_name(m_os.getPlatform())));
    };
}

std::string buildPackageName(std::string stem)
{
    return fmt::format("{}{}", cloyster::productName, stem);
}

std::vector<std::string> getDependenciesEL(const OS& osinfo)
{
    const auto platform = osinfo.getPlatform();
    std::vector<std::string> dependencies;

    std::size_t version = osinfo.getMajorVersion();
    std::string powertools = "powertools";
    switch (platform) {
        case OS::Platform::el8:
        case OS::Platform::el9:
            powertools = "crb";
            break;
        default:
            break;
    };

    switch (osinfo.getDistro()) {
        case OS::Distro::AlmaLinux:
            dependencies
                = { buildPackageName("-AlmaLinux-BaseOS"), powertools };
            break;
        case OS::Distro::RHEL:
            dependencies = { fmt::format(
                "codeready-builder-for-rhel-{}-x86_64-rpms", version) };
            break;
        case OS::Distro::OL:
            dependencies = { buildPackageName("-OL-BaseOS"),
                fmt::format("ol{}_codeready_builder", version) };
            break;
        case OS::Distro::Rocky:
            dependencies = { buildPackageName("-Rocky-BaseOS"), powertools };
            break;
        default:
            throw std::runtime_error(fmt::format("Unsupported distribution: {}",
                magic_enum::enum_name(osinfo.getDistro())));
    }

    return dependencies;
}

void writeSection(
    KeyFile& file, const std::shared_ptr<const RPMRepository>& repo)
{
    std::string section = repo->id();

    file.setString(section, "name", repo->name());
    file.setString(section, "baseurl", repo->baseurl());
    file.setString(section, "metalink", repo->metalink());
    file.setString(section, "enabled",
        static_cast<std::string>(repo->enabled() ? "1" : "0"));
    file.setString(section, "gpgcheck",
        static_cast<std::string>(repo->gpgcheck() ? "1" : "0"));
    file.setString(section, "gpgkey", repo->gpgkey());

    LOG_INFO("writing repo <{}> (id {}) at {}", repo->name(), section,
        repo->source().string());
}

}; // anonymous namespace

namespace cloyster::services::repos {

void RPMRepositoryParser::parse(
    const std::filesystem::path& path, std::vector<RPMRepository>& output)
{
    auto file = KeyFile(path);
    auto reponames = file.getGroups();
    LOG_ASSERT(reponames.size() > 0,
        fmt::format("Empty repository file at {}", path.string()));
    output.reserve(reponames.size());
    for (const auto& repogroup : reponames) {
        auto name = file.getString(repogroup, "name");

        if (name.empty()) {
            throw std::runtime_error(std::format(
                "Could not load repo name from repo '{}'", repogroup));
        }

        auto metalink = file.getStringOpt(repogroup, "metalink");
        auto baseurl = file.getStringOpt(repogroup, "baseurl");
        auto enabled = file.getBoolean(repogroup, "enabled");
        auto gpgcheck = file.getBoolean(repogroup, "gpgcheck");
        auto gpgkey = file.getString(repogroup, "gpgkey");

        RPMRepository repo;
        repo.group(repogroup);
        repo.name(name);
        repo.metalink(metalink);
        repo.baseurl(baseurl);
        repo.enabled(enabled);
        repo.gpgcheck(gpgcheck);
        repo.gpgkey(gpgkey);
        repo.source(path.string());
        repo.id(repogroup);
        repo.valid();
        output.emplace_back(std::move(repo));
    }
}

void RPMRepositoryParser::unparse(
    const std::vector<RPMRepository>& repositories,
    const std::filesystem::path& output)
{
    auto file = cloyster::services::files::KeyFile(output);
    for (const auto& repo : repositories) {
        file.setString(repo.group(), "name", repo.name());
        file.setBoolean(repo.group(), "enabled", repo.enabled());
        file.setBoolean(repo.group(), "gpgcheck", repo.gpgcheck());
        file.setString(repo.group(), "gpgkey", repo.gpgkey());
        file.setString(repo.group(), "metalink", repo.metalink());
        file.setString(repo.group(), "baseurl", repo.metalink());
    }

    file.save();
}

inline void RPMRepository::valid() const
{
    auto isValid = (!id().empty() && !name().empty()
        && (!uri().has_value() || !uri().value().empty())
        && (!source().empty()));
    LOG_ASSERT(isValid, "Invalid RPM Repository");
}

RepoManager::RepoManager(const OS& osinfo)
    : m_os(osinfo)
{
}

void RepoManager::initializeDefaultRepositories()
{
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            loadDefaults(m_os, m_repos, m_filesLoaded);
            loadFiles("/etc/yum.repos.d");
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("DEB packages not implemented");
            break;
    }
}

void RepoManager::loadFiles(const std::filesystem::path& basedir)
{
    for (auto const& dir_entry :
        std::filesystem::directory_iterator { basedir }) {
        const auto& path = dir_entry.path();
        if (path.extension() == ".repo") {
            loadSingleFile(path);
        }
    }

    RepoManager::Repositories cloysterRepos;

    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            cloysterRepos = getDefaultReposFromDisk(basedir);
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("Not implemented deb packages");
            break;
    }
    mergeWithCurrentList(std::move(cloysterRepos));

    auto destparent = std::filesystem::temp_directory_path() / "cloyster0";

    auto destination = destparent / "yum.repos.d";
    cloyster::createDirectory(destination);

    configureEL();
    configureXCAT(destination);
}

void RepoManager::loadSingleFile(const std::filesystem::path& source)
{
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            loadRPMRepos(source, m_repos, m_filesLoaded);
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("DEB packages Not implemented");
            break;
    };
}

void RepoManager::enable(const std::string& repoid)
{
    try {
        m_repos.at(repoid)->enabled(true);
    } catch (const std::out_of_range&) {
        LOG_ERROR("Trying to enable unknown repository {}, "
                  "failed because the repository was not found.");

        for (const auto& [id, _] : m_repos) {
            LOG_ERROR("Repository available: {}", id);
        }

#ifndef NDEBUG
        throw std::runtime_error("Read repository failed");
#endif
    }
}

void RepoManager::enable(const std::vector<std::string>& repoids)
{
    std::ranges::for_each(repoids, [&](const auto& repoid) { enable(repoid); });
}

void RepoManager::disable(const std::string& repoid)
{
    m_repos.at(repoid)->enabled(false);
}

void RepoManager::disable(const std::vector<std::string>& repoids)
{
    std::ranges::for_each(
        repoids, [&](const auto& repoid) { disable(repoid); });
}

void RepoManager::install(const std::filesystem::path& path)
{
    loadSingleFile(path);
}

void RepoManager::install(const std::vector<std::filesystem::path>& paths)
{
    for (const auto& repo : paths) {
        install(repo);
    }
}

/// Private API

std::vector<std::shared_ptr<const IRepository>> RepoManager::listRepos() const
{
    return m_repos | std::views::transform([](const auto& repopair) {
        return repopair.second;
    }) | std::ranges::to<std::vector<std::shared_ptr<const IRepository>>>();
}

void RepoManager::configureEL()
{
    std::vector<std::string> deps = getDependenciesEL(m_os);

    std::ranges::for_each(deps, [&](const auto& repo) { enable(repo); });
}

[[deprecated("RepoManager refactoring fix this!")]]
void RepoManager::saveToDisk()
{
    // @TODO IS THIS CORRECT? BaseRunner should be upcasted here?
    // is runner dynamic dipatching: YES! It is dyn dispatching
    auto runner = getRunner();
    runner->executeCommand("dnf -y install initscripts");
    createFileFor("/etc/yum.repos.d/cloyster.repo");
    auto tmpdir
        = std::filesystem::temp_directory_path() / "cloyster0/yum.repos.d";
    for (auto const& dir_entry : utils::openDir(tmpdir)) {
        cloyster::copyFile(dir_entry, "/etc/yum.repos.d");
    }

    std::vector<std::string> toEnable;
    std::vector<std::string> toDisable;

    for (const auto& [_, repo] : m_repos) {
        if (repo->enabled()) {
            toEnable.push_back(repo->id());
        } else {
            toDisable.push_back(repo->id());
        }
    }

    if (!toEnable.empty()) {
        runner->executeCommand(
            fmt::format("sudo dnf config-manager --set-enabled {}",
                fmt::join(toEnable, ",")));
    }

    if (!toDisable.empty()) {
        runner->executeCommand(
            fmt::format("sudo dnf config-manager --set-disabled {}",
                fmt::join(toDisable, ",")));
    }
}

RepoManager::Repositories RepoManager::getDefaultReposFromDisk(
    const std::filesystem::path& basedir)
{
    RepoManager::Repositories cloyster_repos;

    if (cloyster::customRepofilePath.empty()) {
        loadDefaults(m_os, cloyster_repos, m_filesLoaded);
    } else {
        LOG_INFO("Using custom repofile ({}).", cloyster::customRepofilePath);
        loadRPMRepos(
            cloyster::customRepofilePath, cloyster_repos, m_filesLoaded);
    }

    auto outpath = basedir / "cloyster.repo";
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            loadRPMRepos(outpath, cloyster_repos, m_filesLoaded);
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("Not implemented");
            break;
    }

    return cloyster_repos;
}

void RepoManager::createFileFor(std::filesystem::path path)
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create file {}", path.string());
        return;
    }

    if (std::filesystem::exists(path)) {
        LOG_INFO("Repository file {} already exists...", path.string());
        return;
    }

    auto filtered = m_repos | std::views::filter([&path](const auto& r) {
        return path == r.second->source();
    });

    for (const auto& [_, repo] : filtered) {
        files::KeyFile file(repo->source());

        switch (m_os.getPackageType()) {
            case OS::PackageType::RPM:
                writeSection(
                    file, dynamic_pointer_cast<const RPMRepository>(repo));
                break;
            case OS::PackageType::DEB:
                throw std::logic_error("Not implemented");
                break;
            default:
                throw std::runtime_error("Unknown package type");
        }

        file.save();
    }
}

void RepoManager::mergeWithCurrentList(Repositories&& repos)
{
    for (auto&& [key, repo] : repos) {
        m_repos.insert({ key, repo });
    }
}

static void createGPGKey(
    const std::filesystem::path& path, const std::string& key)
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create GPG Key file {}", path.filename().string())
        return;
    }

    std::ofstream gpgkey(path);
    gpgkey << key;
    gpgkey.close();
}

// @TODO: Shouldn't this be at XCAT class ?
void RepoManager::configureXCAT(const std::filesystem::path& repofileDest)
{
    LOG_INFO("Setting up XCAT repositories");
    auto runner = cloyster::getRunner();

    // TODO: we need to download these files in a sort of temporary directory
    runner->downloadFile("https://xcat.org/files/xcat/repos/yum/devel/"
                         "core-snap/xcat-core.repo",
        repofileDest.string());

    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            runner->downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh8/x86_64/xcat-dep.repo",
                repofileDest.string());
            break;
        case OS::Platform::el9:
#ifndef NDEBUG
        // Hack to test EL10 only in debug mode
        case OS::Platform::el10:
#endif
            runner->downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh9/x86_64/xcat-dep.repo",
                repofileDest.string());
            break;
        default:
            throw std::runtime_error("Unsupported platform for xCAT");
    }

    // Do not visit repodfileDest if we're in dryRun
    // because the folder will not exist
    if (cloyster::dryRun) {
        LOG_WARN(
            "Dry Run: Would load repositories at {}", repofileDest.string());
        return;
    }

    LOG_DEBUG("Visiting {}", repofileDest.string());
    for (auto const& dirEntry :
        std::filesystem::directory_iterator { repofileDest }) {
        const auto& path = dirEntry.path();
        if (path.extension() == ".repo") {
            loadSingleFile(path);
        }
    }
}

}; // namespace cloyster::services::repos

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <cloysterhpc/tempdir.h>
#include <cloysterhpc/tests.h>

#ifdef BUILD_TESTING
TEST_SUITE("Test repository file read and write")
{
    TEST_CASE(
        "Check if the default cloyster repository file is correctly parsed")
    {
        MockRunner mr;
        OS osinfo { OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el9,
            OS::Distro::Rocky, "6.69.6969", 9, 9 };

        /*
        RepoManager repo { mr, osinfo };
        repo.loadFiles(tests::sampleDirectory);

        auto rlist = repo.listRepos();

        REQUIRE(rlist.size() == 16);

        std::sort(rlist.begin(), rlist.end(), [](auto a, auto b) {
            return (a.id <=> b.id) == std::strong_ordering::less;
        });

        CHECK(rlist[0].id == "CloysterHPC-AlmaLinux-BaseOS");
        CHECK(rlist[0].name == "AlmaLinux $releasever - BaseOS");
        CHECK(rlist[0].baseurl
            == "https://repo.almalinux.org/almalinux/$releasever/BaseOS/"
               "$basearch/os/");
        CHECK(rlist[5].id == "CloysterHPC-epel");
        CHECK(rlist[5].name
            == "Extra Packages for Enterprise Linux 9 - $basearch");
        CHECK(rlist[5].baseurl
            == "https://mirror.versatushpc.com.br/epel/9/Everything/x86_64/");
        CHECK(rlist[15].id == "CloysterHPC-zabbix");
        CHECK(rlist[15].name == "zabbix");
        CHECK(rlist[15].baseurl
            == "https://mirror.versatushpc.com.br/zabbix/zabbix/6.5/rhel/9/"
               "x86_64/");
           */
    }

    TEST_CASE("Check if the repository enable operations are run")
    {
        /*
        MockRunner mr;
        OS osinfo { OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el9,
            OS::Distro::Rocky, "6.69.6969", 9, 9 };

        TempDir d;

        RepoManager repo { mr, osinfo };
        repo.loadFiles(d.name());

        auto rlist = repo.listRepos();

        REQUIRE(rlist.size() == 16);

        std::sort(rlist.begin(), rlist.end(), [](auto a, auto b) {
            return (a.id <=> b.id) == std::strong_ordering::less;
        });
        */
    }
}
#endif
