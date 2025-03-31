/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <filesystem>
#include <functional>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>

using cloyster::OS;
using cloyster::concepts::IsParser;
using cloyster::services::files::IsKeyFileReadable;
using cloyster::services::files::KeyFile;
using cloyster::services::repos::IRepository;
using std::make_unique;

namespace cloyster::services::repos {

// Represents a debian repository file
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

// Represents a RPM Repository inside a repository file (/etc/yum.repo.d/*.repo)
class RPMRepository final : public IRepository {
    std::string m_id;
    bool m_enabled = true;
    std::string m_name;
    std::optional<std::string> m_baseurl;
    std::optional<std::string> m_metalink;
    std::optional<std::string> m_gpgkey;
    bool m_gpgcheck = true;
    std::filesystem::path m_source;
    std::string m_group;

public:
    RPMRepository() = default;
    ~RPMRepository() override = default;
    RPMRepository(const RPMRepository&) = default;
    RPMRepository(RPMRepository&&) = default;
    RPMRepository& operator=(const RPMRepository&) = default;
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
    [[nodiscard]] std::optional<std::string> gpgkey() const { return m_gpgkey; };

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
    void gpgkey(std::optional<std::string> gpgkey) { m_gpgkey = std::move(gpgkey); };

    void valid() const;

    bool operator==(const auto other) const { return other.id() == id(); }
};

namespace {

    // Easy conversions for string
    std::ostream& operator<<(std::ostream& ostr, const RPMRepository& repo)
    {
        ostr << "RPMRepository(";
        ostr << repo.id() << " ";
        ostr << repo.name() << " ";
        ostr << repo.baseurl().value_or("") << " ";
        ostr << "enabled=" << repo.enabled() << " ";
        ostr << ")";

        return ostr;
    };

    template <typename T> std::string toString(const T& input)
    {
        std::ostringstream strm;
        strm << input;
        return strm.str();
    };

};

// Parses RPM .repo files
class RPMRepositoryParser final {
public:
    static void parse(const std::filesystem::path& path,
        std::unordered_map<std::string, std::shared_ptr<RPMRepository>>& output)
    {
        LOG_DEBUG("Parsing file {}", path.string());
        auto file = KeyFile(path);
        auto reponames = file.getGroups();
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
            auto gpgkey = file.getStringOpt(repogroup, "gpgkey");

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

            output.emplace(repo.id(), std::make_shared<RPMRepository>(repo));
        }
    }

    static void unparse(
        const std::unordered_map<std::string, std::shared_ptr<RPMRepository>>&
            repos,
        const std::filesystem::path& path)
    {
        auto file = cloyster::services::files::KeyFile(path);
        for (const auto& [repoId, repo] : repos) {
            file.setString(repo->group(), "name", repo->name());
            file.setBoolean(repo->group(), "enabled", repo->enabled());
            file.setBoolean(repo->group(), "gpgcheck", repo->gpgcheck());
            file.setString(repo->group(), "gpgkey", repo->gpgkey());
            file.setString(repo->group(), "metalink", repo->metalink());
            file.setString(repo->group(), "baseurl", repo->baseurl());
        }

        file.save();
    }
};
static_assert(IsParser<RPMRepositoryParser, std::filesystem::path,
    std::unordered_map<std::string, std::shared_ptr<RPMRepository>>>);

// Represents a file inside /etc/yum.repos.d
class RPMRepositoryFile final {
    static constexpr auto m_parser = RPMRepositoryParser();
    std::filesystem::path m_path;
    std::unordered_map<std::string, std::shared_ptr<RPMRepository>> m_repos;

public:
    explicit RPMRepositoryFile(auto path)
        : m_path(std::move(path))
    {
        m_parser.parse(m_path, m_repos);
    }

    const auto& path() { return m_path; }

    auto& repos() { return m_repos; }

    auto repo(const std::string& name) { return m_repos.at(name); }

    void save()
    {
        LOG_DEBUG("Saving {}", m_path.string());
        m_parser.unparse(m_repos, m_path);
    }
};

// Installs and enable/disable RPM repositories
class RPMRepoManager final {
    static constexpr auto m_parser = RPMRepositoryParser();
    // Maps repo id to files
    std::unordered_map<std::string, std::shared_ptr<RPMRepositoryFile>>
        m_filesIdx;

public:
    static constexpr auto basedir = "/etc/yum.repos.d/";

    // Installs a single .repo file
    void install(const std::filesystem::path& source)
    {
        const auto& dest = basedir / source.filename();

        // Do not copy the file to the basedir if it
        // is already there
        if (source != dest) {
            cloyster::copyFile(source, dest);
        }

        if (cloyster::dryRun) {
            LOG_INFO("Dry Run: Would open {}", dest.string());
            return;
        }

        LOG_DEBUG("Installing repository {}", dest.string());
        const auto& repofile
            = std::make_shared<RPMRepositoryFile>(RPMRepositoryFile(dest));
        LOG_ASSERT(repofile->repos().size() > 0, "BUG Loading file");
        for (auto& [repo, _] : repofile->repos()) {
            LOG_DEBUG("{} loaded", repo);
            m_filesIdx.emplace(repo, repofile);
        }
    }

    // Install all .repo files inside a folder
    void install(std::filesystem::directory_iterator&& dirIter)
    {
        for (const auto& fil : std::move(dirIter)) {
            std::string fname = fil.path().filename().string();
            if (fname.ends_with(".repo")) {
                install(fil);
            }
        }
    }

    // Install all .repos files inside a folder
    void installDir(const std::filesystem::path& path)
    {
        if (cloyster::dryRun) {
            LOG_INFO("Dry Run: Would open the directory {}", path.string());
            return;
        }

        install(std::filesystem::directory_iterator(path));
    }

    void installBaseDir() { installDir(basedir); }

    auto repo(const std::string& repoName)
    {
        try {
            auto repoFile = m_filesIdx.at(repoName);
            auto repoObj = repoFile->repo(repoName);
            return std::make_unique<const RPMRepository>(*repoObj); // copy to unique ptr
        } catch (const std::out_of_range& e) {
            auto repos = m_filesIdx | std::views::transform([](const auto& pair){
                return pair.first;
            });
            auto msg = fmt::format(
                "Cannot enable repository {}, no such repository loaded, repositories: available: {}",
                repoName,
                fmt::join(repos, ","));
            throw std::runtime_error(msg);
        }
    }

    // Enable/diable a repository by name
    void enable(const auto& repo, bool value)
    {
        LOG_DEBUG("Enabling/Disabling[{}] RPM repo {}", value, repo);
        auto& repofile = m_filesIdx.at(repo);
        repofile->repo(repo)->enabled(value);
        repofile->save();
    }

    // Enable a repo but dot not save the repofile, (used internally)
    void enable(const auto& repo, auto& repofile, bool value)
    {
        LOG_DEBUG("Enabling/Disabling[{}] RPM repo {}", value, repo);
        repofile->repo(repo)->enabled(value);
    }

    // Enable/disable multiple repositories by name
    void enable(const std::vector<std::string>& repos, bool value)
    {
        auto byIdPtr = [](const std::shared_ptr<RPMRepositoryFile>& rptr) {
            return std::hash<std::string> {}(rptr->path());
        };
        std::unordered_set<std::shared_ptr<RPMRepositoryFile>,
            decltype(byIdPtr)>
            toSave;
        for (const auto& repo : repos) {
            auto& rfile = m_filesIdx.at(repo);
            toSave.emplace(rfile);
            enable(repo, rfile, value);
        }
        for (const auto& repoFil : toSave) {
            repoFil->save();
        }
    }

    // List repositories through a const unique pointer vector
    //
    // Rationale: IRepository type is to keep client code generic
    std::vector<std::unique_ptr<const IRepository>> repos()
    {
        // Function to iterate over map by id
        constexpr auto byId
            = [](auto& repo) { return std::hash<std::string>{}(repo.id()); };

        std::unordered_set<RPMRepository, decltype(byId)> output;
        for (auto& [_id1, repoFile] : m_filesIdx) {
            for (const auto& [_id2, repo] : repoFile->repos()) {
                output.emplace(*repo); // copy
            }
        }

        return output | std::views::transform([](auto&& repo) {
            return std::make_unique<const RPMRepository>(repo);
        }) | std::ranges::to<std::vector<std::unique_ptr<const IRepository>>>();
    }
};

}; // namespace cloyster::services::repos {

namespace {

// These files contains the repositories files data as
// format strings with which we inject the values for
constexpr std::string_view CLOYSTER_REPO_EL8 = {
#include "cloysterhpc/repos/el8/cloyster.repo"
};

constexpr std::string_view CLOYSTER_REPO_EL9 = {
#include "cloysterhpc/repos/el9/cloyster.repo"
};

constexpr std::string_view CLOYSTER_REPO_EL10 = {
#include "cloysterhpc/repos/el10/cloyster.repo"
};

constexpr auto getCloysterRepoPath(const auto& osinfo)
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

inline std::string buildPackageName(std::string stem)
{
    return fmt::format("{}{}", cloyster::productName, stem);
}

// Returns a list of repositories to be enabled
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
                cloyster::utils::enums::toString(osinfo.getDistro())));
    }

    return dependencies;
}

}; // anonymous namespace

namespace cloyster::services::repos {

// Hidden implementation
struct RepoManager::Impl {
    RPMRepoManager rpm;
    // Add debian repo manager here when the day arrives
};

RepoManager::~RepoManager() = default;

RepoManager::RepoManager(const OS& osinfo)
    : m_impl(std::make_unique<RepoManager::Impl>())
    , m_os(osinfo)
{
}

// Installs the cloyster.repo file and return its path
std::filesystem::path RepoManager::generateCloysterReposFile()
{
    const auto& path = getCloysterRepoPath(m_os);
    std::istringstream stream;
    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL8, cloyster::productName));
            break;
        case OS::Platform::el9:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL9, cloyster::productName));
            break;
        case OS::Platform::el10:
            stream = std::istringstream(
                fmt::format(CLOYSTER_REPO_EL10, cloyster::productName));
            break;
        default:
            throw std::runtime_error(fmt::format("Unsupported platform {}",
                cloyster::utils::enums::toString(m_os.getPlatform())));
    };

    cloyster::installFile(path, stream);
    return path;
}

inline void RPMRepository::valid() const
{
    auto isValid = (!id().empty() && !name().empty()
        && (!uri().has_value() || !uri().value().empty())
        && (!source().empty()));
    LOG_ASSERT(isValid, "Invalid RPM Repository");
}

void RepoManager::initializeDefaultRepositories()
{
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            m_impl->rpm.installBaseDir();
            m_impl->rpm.install(generateCloysterReposFile());
            // enable() is dryRun aware
            enable(getDependenciesEL(m_os));
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("DEB packages not implemented");
            break;
    }
}

void RepoManager::enable(const std::string& repoid)
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", repoid);
        return;
    }

    try {
        switch (m_os.getPackageType()) {
            case OS::PackageType::RPM:
                m_impl->rpm.enable(repoid, true);
                break;
            default:
                throw std::logic_error("Not implemented");
        }
    } catch (const std::out_of_range&) {
        LOG_ERROR("Trying to enable unknown repository {}, "
                  "failed because the repository was not found.",
            repoid);
    }
}

void RepoManager::enable(const std::vector<std::string>& repos)
{
    if (cloyster::dryRun) {
        LOG_WARN(
            "Dry Run: Would enable these repos: {}", fmt::join(repos, ","));
        return;
    }
    try {
        switch (m_os.getPackageType()) {
            case OS::PackageType::RPM:
                m_impl->rpm.enable(repos, true);
                break;
            default:
                throw std::logic_error("Not implemented");
        }
    } catch (const std::out_of_range&) {
        LOG_ERROR("Trying to enable unknown repository {}, "
                  "failed because the repository was not found.",
            fmt::join(repos, ","));
    }
}

void RepoManager::disable(const std::string& repoid)
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", repoid);
        return;
    }

    try {
        switch (m_os.getPackageType()) {
            case OS::PackageType::RPM:
                m_impl->rpm.enable(repoid, false);
                break;
            default:
                throw std::logic_error("Not implemented");
        }
    } catch (const std::out_of_range&) {
        LOG_ERROR("Trying to disable unknown repository {}, "
                  "failed because the repository was not found.",
            repoid);
    }
}

void RepoManager::disable(const std::vector<std::string>& repos)
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", fmt::join(repos, ","));
        return;
    }

    try {
        switch (m_os.getPackageType()) {
            case OS::PackageType::RPM:
                m_impl->rpm.enable(repos, false);
                break;
            default:
                throw std::logic_error("Not implemented");
        }
    } catch (const std::out_of_range&) {
        LOG_ERROR("Trying to disable unknown repository {}, "
                  "failed because the repository was not found.",
            fmt::join(repos, ","));
    }
}

void RepoManager::install(const std::filesystem::path& path)
{
    LOG_ASSERT(
        path.is_absolute(), "RepoManager::install called with relative path");
    LOG_ASSERT(
        path.has_filename(), "RepoManager::install called with a directory?");
    LOG_INFO("Installing repository {}", path.string());
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            m_impl->rpm.install(path);
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

void RepoManager::install(const std::vector<std::filesystem::path>& paths)
{
    for (const auto& repo : paths) {
        install(repo);
    }
}

// Return a vector of repositories.
//
// Pay attention that the returned value is an abstract type IRepository. This
// is to keep client code agostic to the repository implementations. They
// must dynamic dispatch over the shared pointer in order to treat the
// repository abstractly.
std::vector<std::unique_ptr<const IRepository>> RepoManager::listRepos() const
{
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            return m_impl->rpm.repos();
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

std::unique_ptr<const IRepository> RepoManager::repo(const std::string& repo) const
{
    switch (m_os.getPackageType()) {
        case OS::PackageType::RPM:
            return static_cast<std::unique_ptr<const IRepository>>(m_impl->rpm.repo(repo));
            break;
        default:
            throw std::logic_error("Not implemented");
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
