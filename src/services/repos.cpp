/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <gsl/gsl-lite.hpp>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>

using cloyster::OS;
using cloyster::concepts::IsParser;
using cloyster::services::files::IsKeyFileReadable;
using cloyster::services::files::KeyFile;
using cloyster::services::repos::IRepository;

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
    [[nodiscard]] std::optional<std::string> gpgkey() const
    {
        return m_gpgkey;
    };

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
    void gpgkey(std::optional<std::string> gpgkey)
    {
        m_gpgkey = std::move(gpgkey);
    };

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
        std::map<std::string, std::shared_ptr<RPMRepository>>& output)
    {
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
        const std::map<std::string, std::shared_ptr<RPMRepository>>&
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
    std::map<std::string, std::shared_ptr<RPMRepository>>>);

// Represents a file inside /etc/yum.repos.d
class RPMRepositoryFile final {
    static constexpr auto m_parser = RPMRepositoryParser();
    std::filesystem::path m_path;
    std::map<std::string, std::shared_ptr<RPMRepository>> m_repos;

public:
    explicit RPMRepositoryFile(auto path)
        : m_path(std::move(path))
    {
        m_parser.parse(m_path, m_repos);
    }

    RPMRepositoryFile(std::filesystem::path path,
        std::map<std::string, std::shared_ptr<RPMRepository>> repos)
        : m_path(std::move(path))
        , m_repos(std::move(repos))
    {
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
    std::map<std::string, std::shared_ptr<RPMRepositoryFile>>
        m_filesIdx;

public:
    static constexpr auto basedir = "/etc/yum.repos.d/";

    // Installs a single .repo file
    void install(const std::filesystem::path& source)
    {
        const auto& dest = basedir / source.filename();
        const auto opts
            = cloyster::Singleton<cloyster::services::Options>::get();

        // Do not copy the file to the basedir if it
        // is already there
        if (source != dest) {
            cloyster::copyFile(source, dest);
        }

        if (opts->dryRun) {
            LOG_INFO("Dry Run: Would open {}", dest.string());
            return;
        }

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
                LOG_DEBUG("Loading {}", fname);
                install(fil);
            }
        }
    }

    // Install all .repos files inside a folder
    void loadDir(const std::filesystem::path& path)
    {
        const auto opts
            = cloyster::Singleton<cloyster::services::Options>::get();
        if (opts->dryRun) {
            LOG_INFO("Dry Run: Would open the directory {}", path.string());
            return;
        }

        install(std::filesystem::directory_iterator(path));
    }

    void loadBaseDir() { loadDir(basedir); }

    auto repo(const std::string& repoName)
    {
        try {
            auto repoFile = m_filesIdx.at(repoName);
            auto repoObj = repoFile->repo(repoName);
            return std::make_unique<const RPMRepository>(
                *repoObj); // copy to unique ptr
        } catch (const std::out_of_range& e) {
            auto repos = m_filesIdx
                | std::views::transform(
                    [](const auto& pair) { return pair.first; });
            auto msg
                = fmt::format("Cannot find repository {}, no such repository "
                              "loaded, repositories: available: {}",
                    repoName, fmt::join(repos, ","));
            throw std::runtime_error(msg);
        }
    }

    static std::vector<std::unique_ptr<const IRepository>> repoFile(const std::string& repoFileName)
    {
        try {
            auto path = fmt::format("{}/{}", basedir, repoFileName);
            auto repos = RPMRepositoryFile(path).repos() 
                // We copy to cons unique to express that these values cannot
                // be changed through this API
                | std::views::transform([](auto&& pair) { return std::make_unique<const RPMRepository>(*pair.second); })
                | std::ranges::to<std::vector<std::unique_ptr<const IRepository>>>();
            return repos;
        } catch (const std::out_of_range& e) {
            throw std::runtime_error(
                fmt::format("No such repository file {}", repoFileName));
        }
    }

    // Enable/disable a repository by name
    void enable(const auto& repo, bool value)
    {
        if (value) {
            LOG_DEBUG("Enabling RPM repo {}", repo);
        } else {
            LOG_DEBUG("Disabling RPM repo {}", repo);
        }
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
            = [](auto& repo) { return std::hash<std::string> {}(repo.id()); };

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

namespace cloyster::services::repos {

// Hidden implementation
struct RepoManager::Impl {
    RPMRepoManager rpm;
    // Add debian repo manager here when the day arrives
};

RepoManager::~RepoManager() = default;

RepoManager::RepoManager()
    : m_impl(std::make_unique<RepoManager::Impl>())
{
}

inline void RPMRepository::valid() const
{
    auto isValid = (!id().empty() && !name().empty()
        && (!uri().has_value() || !uri().value().empty())
        && (!source().empty()));
    LOG_ASSERT(isValid, "Invalid RPM Repository");
}

struct ELConfig {
    static inline const std::filesystem::path baseDir = "/etc/yum.repos.d/";

    // @TODO: Extract the upstream URLs prefixes to the command line options with 
    //   the current values as defaults
    static std::vector<RPMRepositoryFile> getRepositories()
    {
        const auto& osinfo = cloyster::Singleton<models::Cluster>::get()
            ->getHeadnode()
            .getOS();
        std::string releasever
            = std::to_string(osinfo.getMajorVersion()); // e.g., "8"
        std::string arch = cloyster::utils::enums::toString(
            osinfo.getArch()); // e.g., "x86_64"
        const auto opts
            = cloyster::Singleton<cloyster::services::Options>::get();
        auto distro = osinfo.getDistro(); // Adjust based on your actual API

        std::vector<RPMRepositoryFile> repoFiles;

        auto addRepo = [](std::map<std::string,
                          std::shared_ptr<RPMRepository>>& repoMap,
                          // NOLINTNEXTLINE
                          const std::string& id, const std::string& name,
                          const std::string& baseurl, bool enabled,
                          const std::string& gpgkey) {
                auto repo = std::make_shared<RPMRepository>();
                repo->group(id);
                repo->id(id);
                repo->name(name);
                repo->baseurl(baseurl);
                repo->enabled(enabled);
                repo->gpgcheck(!gpgkey.empty());
                repo->gpgkey(gpgkey);
                repoMap.emplace(id, repo);
            };

        auto addRepoFile = [&repoFiles, &opts](const auto& path, auto&& repos) {
            if (opts->shouldForce("generate-repos") || !cloyster::exists(path)) {
                LOG_INFO("Generating {}", path.string());
                cloyster::removeFile(path.string());
                repoFiles.emplace_back(path, repos);
            } else {
                LOG_WARN("Skipping the  {}, file exists, use --force=generate-repos to force", path.string());
            }
        };

        // rhel.repo (Added for RHEL)
        if (distro == OS::Distro::RHEL) {
            std::map<std::string, std::shared_ptr<RPMRepository>> rhelRepos;
            // Add RHEL CodeReady Linux Builder
            // Upstream URL: https://cdn.redhat.com/content/dist/rhel8/x86_64/codeready-builder/os/
            addRepo(rhelRepos, "rhel-" + releasever + "-codeready-builder",
                    "Red Hat Enterprise Linux " + releasever + " - CodeReady Builder (" + arch + ")",
                    EnterpriseLinux::repositoryURL("RHELCodeReady",
                                  "rhel/" + releasever + "/" + arch + "/codeready-builder/os/",
                                  "https://cdn.redhat.com/content/dist/rhel/" + releasever + "/" + arch + "/codeready-builder/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("RHELCodeReady", "RPM-GPG-KEY-redhat-release",
                                  "https://www.redhat.com/security/data/fd431d51.txt"));

            // Upstream URL: https://cdn.redhat.com/content/dist/rhel8/x86_64/baseos/os/
            addRepo(rhelRepos, "rhel-" + releasever + "-baseos",
                    "Red Hat Enterprise Linux " + releasever + " - BaseOS (" + arch + ")",
                    EnterpriseLinux::repositoryURL("RHELBaseOS",
                                  "rhel/" + releasever + "/" + arch + "/baseos/os/",
                                  "https://cdn.redhat.com/content/dist/rhel/" + releasever + "/" + arch + "/baseos/os/"),
                    true,
                    EnterpriseLinux::repositoryURL("RHELBaseOS", "RPM-GPG-KEY-redhat-release",
                                  "https://www.redhat.com/security/data/fd431d51.txt"));

            addRepoFile(baseDir / "rhel.repo", std::move(rhelRepos));
        }

        // oracle.repo
        if (distro == OS::Distro::OL) {
            std::map<std::string, std::shared_ptr<RPMRepository>> oracleRepos;
            // Add Oracle Linux CodeReady Builder
            // Upstream URL: https://yum.oracle.com/repo/OracleLinux/OL8/codeready/builder/x86_64/
            addRepo(oracleRepos, "ol" + releasever + "_codeready_builder",
                    "Oracle Linux " + releasever + " CodeReady Builder (" + arch + ")",
                    EnterpriseLinux::repositoryURL("OLCodeReady",
                                  "repo/OracleLinux/OL" + releasever + "/codeready/builder/" + arch + "/",
                                  "https://yum.oracle.com/repo/OracleLinux/OL" + releasever
                                  + "/codeready/builder/" + arch),
                    false,
                    EnterpriseLinux::repositoryURL("OLCodeReady", "RPM-GPG-KEY-oracle-ol" + releasever,
                                  "https://yum.oracle.com/RPM-GPG-KEY-oracle-ol" + releasever));

            // Upstream URL: https://yum.$ociregion.oracle.com/repo/OracleLinux/OL8/baseos/latest/x86_64/
            addRepo(oracleRepos, "OLBaseOS",
                    "Oracle Linux " + releasever + " BaseOS Latest (" + arch + ")",
                    EnterpriseLinux::repositoryURL("OLBaseOS",
                                  "repo/OracleLinux/OL" + releasever + "/baseos/latest/" + arch + "/",
                                  "https://yum.$ociregion.oracle.com/repo/OracleLinux/OL"
                                  + releasever + "/baseos/latest/" + arch + "/"),
                    true,
                    EnterpriseLinux::repositoryURL("OLBaseOS", "RPM-GPG-KEY-oracle-ol" + releasever,
                                  "https://yum.oracle.com/RPM-GPG-KEY-oracle-ol" + releasever));

            addRepoFile(baseDir / "oracle.repo", std::move(oracleRepos));
        }

        // rocky.repo
        if (distro == OS::Distro::Rocky) {
            std::map<std::string, std::shared_ptr<RPMRepository>> rockyRepos;

            bool useVault = services::RockyLinux::shouldUseVault(osinfo.getVersion());

            const std::string& upstreamBaseUrl = (useVault || opts->shouldForce("use-vault"))
                ? "https://dl.rockylinux.org/vault/rocky/" 
                : "https://dl.rockylinux.org/pub/rocky/";

            // Add Rocky Linux CRB/PowerTools based on version
            std::string crbId = (releasever == "8") ? "powertools" : "crb";
            std::string crbName = (releasever == "8") ? "PowerTools" : "CRB";

            // Rocky BaseOS repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/BaseOS/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/BaseOS/x86_64/os/
            addRepo(rockyRepos, "RockyBaseOS",
                    "Rocky Linux $releasever - BaseOS",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/BaseOS/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/BaseOS/" + arch + "/os/"),
                    true,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky AppStream repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/AppStream/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/AppStream/x86_64/os/
            addRepo(rockyRepos, "RockyAppStream",
                    "Rocky Linux $releasever - AppStream",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/AppStream/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/AppStream/" + arch + "/os/"),
                    true,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));
            // Rocky Extras repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/extras/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/extras/x86_64/os/
            addRepo(rockyRepos, "RockyExtras",
                    "Rocky Linux $releasever - Extras",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/extras/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/extras/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky Devel repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/devel/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/devel/x86_64/os/
            addRepo(rockyRepos, "RockyDevel",
                    "Rocky Linux $releasever - Devel",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/devel/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/devel/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky Devel repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/devel/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/devel/x86_64/os/
            addRepo(rockyRepos, "RockyDevel",
                    "Rocky Linux $releasever - Devel",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/devel/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/devel/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));


            // Rocky HighAvailability repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/HighAvailability/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/HighAvailability/x86_64/os/
            addRepo(rockyRepos, "RockyHighAvailability",
                    "Rocky Linux $releasever - HighAvailability",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/HighAvailability/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/HighAvailability/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky ResilientStorage repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/ResilientStorage/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/ResilientStorage/x86_64/os/
            addRepo(rockyRepos, "RockyResilientStorage",
                    "Rocky Linux $releasever - ResilientStorage",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/ResilientStorage/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/ResilientStorage/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky NFV repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/NFV/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/NFV/x86_64/os/
            addRepo(rockyRepos, "RockyNFV",
                    "Rocky Linux $releasever - NFV",
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/NFV/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/NFV/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Rocky CRB/PowerTools repo
            // Upstream URL: https://dl.rockylinux.org/pub/rocky/8/PowerTools/x86_64/os/
            // or https://dl.rockylinux.org/vault/rocky/8/PowerTools/x86_64/os/
            addRepo(rockyRepos, crbId,
                    "Rocky Linux $releasever - " + crbName,
                    EnterpriseLinux::repositoryURL("rocky",
                                  (useVault ? "vault/" : "linux/") + releasever + "/" + crbName + "/" + arch + "/os/",
                                  upstreamBaseUrl + releasever + "/" + crbName + "/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("rocky", "linux/RPM-GPG-KEY-Rocky-" + releasever,
                                  upstreamBaseUrl + "RPM-GPG-KEY-Rocky-" + releasever));

            // Write the repo file
            addRepoFile(baseDir / "rocky.repo", std::move(rockyRepos));
        }

        // almalinux.repo
        if (distro == OS::Distro::AlmaLinux) {
            std::map<std::string, std::shared_ptr<RPMRepository>> almaRepos;
            // Add AlmaLinux CRB/PowerTools based on version
            std::string crbId = (releasever == "8") ? "powertools" : "crb";
            std::string crbName = (releasever == "8") ? "PowerTools" : "CRB";

            // Upstream URL: https://repo.almalinux.org/almalinux/8/PowerTools/x86_64/os/
            addRepo(almaRepos, crbId,
                    "AlmaLinux $releasever - " + crbName,
                    EnterpriseLinux::repositoryURL("Alma" + crbName,
                                  "almalinux/" + releasever + "/" + crbName + "/" + arch + "/os/",
                                  "https://repo.almalinux.org/almalinux/" + releasever + "/" + crbName + "/" + arch + "/os/"),
                    false,
                    EnterpriseLinux::repositoryURL("Alma" + crbName, "RPM-GPG-KEY-AlmaLinux-" + releasever,
                                  "https://repo.almalinux.org/almalinux/RPM-GPG-KEY-AlmaLinux-" + releasever));

            // Upstream URL: https://repo.almalinux.org/almalinux/8/BaseOS/x86_64/os/
            addRepo(almaRepos, "AlmaLinuxBaseOS",
                    "AlmaLinux $releasever - BaseOS",
                    EnterpriseLinux::repositoryURL("AlmaLinuxBaseOS",
                                  "almalinux/" + releasever + "/BaseOS/" + arch + "/os/",
                                  "https://repo.almalinux.org/almalinux/" + releasever + "/BaseOS/" + arch + "/os/"),
                    true,
                    EnterpriseLinux::repositoryURL("AlmaLinuxBaseOS", "RPM-GPG-KEY-AlmaLinux-" + releasever,
                                  "https://repo.almalinux.org/almalinux/RPM-GPG-KEY-AlmaLinux-" + releasever));

            addRepoFile(baseDir / "almalinux.repo", std::move(almaRepos));
        }

        // beegfs.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            beegfsRepos;
            auto path = opts->beegfsVersion + "/dists/rhel" + releasever + "/";
            // Upstream URL: https://www.beegfs.io/release/beegfs_7_3/dists/rhel8/
            addRepo(beegfsRepos, "beegfs", "BeeGFS",
                    EnterpriseLinux::repositoryURL("beegfs", path, "https://www.beegfs.io/release/" + opts->beegfsVersion + "/dists/rhel" + releasever + "/"),
                    false,
                    EnterpriseLinux::repositoryURL("beegfs",
                                  opts->beegfsVersion + "/gpg/GPG-KEY-beegfs",
                                  "https://www.beegfs.io/release/" + opts->beegfsVersion + "/gpg/GPG-KEY-beegfs"));
            addRepoFile(
                baseDir / "beegfs.repo", std::move(beegfsRepos));
        }

        // grafana.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            grafanaRepos;
            // Upstream URL: https://rpm.grafana.com
            addRepo(grafanaRepos, "grafana", "grafana",
                    EnterpriseLinux::repositoryURL("grafana", "", "https://rpm.grafana.com"),
                    false,
                    // There is no GPG key in the mirror, force the upstream key
                    EnterpriseLinux::repositoryURL(
                    "grafana", "gpg.key", "https://rpm.grafana.com/gpg.key", true));
            addRepoFile(
                baseDir / "grafana.repo", std::move(grafanaRepos));
        }

        // influxdata.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            influxRepos;
            // Upstream URL: https://repos.influxdata.com/rhel/8/x86_64/stable/
            addRepo(influxRepos, "influxdata", "InfluxData Repository - Stable",
                    EnterpriseLinux::repositoryURL(
                        "influxdata", "rhel/" + releasever + "/" + arch + "/stable/",
                        "https://repos.influxdata.com/rhel/" + releasever + "/" + arch + "/stable",
                        // skip mirror for influxdata
                        true),
                    false,
                    EnterpriseLinux::repositoryURL("influxdata", "influxdata-archive_compat.key",
                                  "https://repos.influxdata.com/influxdata-archive_compat.key", true));
            addRepoFile(
                baseDir / "influxdata.repo", std::move(influxRepos));
        }

        // intel.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            intelRepos;
            // Upstream URL: https://yum.repos.intel.com/oneAPI
            addRepo(intelRepos, "oneAPI", "Intel oneAPI repository",
                    EnterpriseLinux::repositoryURL("oneAPI", "", "https://yum.repos.intel.com/oneapi"),
                    false,
                    EnterpriseLinux::repositoryURL("oneAPI",
                                  "GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB",
                                  "https://yum.repos.intel.com/intel-gpg-keys/"
                                  "GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB"));
            addRepoFile(
                baseDir / "intel.repo", std::move(intelRepos));
        }

        // zabbix.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>> zabbixRepos;
            // Upstream URL: https://repo.zabbix.com/zabbix/6.4/rhel/8/x86_64/
            addRepo(zabbixRepos, "zabbix", "zabbix",
                    EnterpriseLinux::repositoryURL("zabbix",
                                  "zabbix/" + opts->zabbixVersion + "/rhel/" + releasever + "/" + arch + "/",
                                  "https://repo.zabbix.com/zabbix/" + opts->zabbixVersion + "/rhel/"
                                  + releasever + "/" + arch),
                    false,
                    EnterpriseLinux::repositoryURL("zabbix", "RPM-GPG-KEY-ZABBIX",
                                  "https://repo.zabbix.com/RPM-GPG-KEY-ZABBIX"));
            addRepoFile(
                baseDir / "zabbix.repo", std::move(zabbixRepos));
        }

        // elrepo.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            elrepoRepos;
            // Upstream URL: https://elrepo.org/linux/elrepo/el8/x86_64/
            addRepo(elrepoRepos, "elrepo", "elrepo",
                    EnterpriseLinux::repositoryURL("elrepo",
                                  "elrepo/el" + releasever + "/" + arch + "/",
                                  "https://elrepo.org/linux/elrepo/el" + releasever + "/"
                                  + arch),
                    false,
                    EnterpriseLinux::repositoryURL("elrepo", "RPM-GPG-KEY-elrepo.org",
                                  "https://www.elrepo.org/RPM-GPG-KEY-elrepo.org"));
            addRepoFile(
                baseDir / "elrepo.repo", std::move(elrepoRepos));
        }

        // rpmfusion.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            rpmfusionRepos;
            // Upstream URL: https://download1.rpmfusion.org          /free/el/updates/8/x86_64/
            // Mirror URL: https://mirror.versatushpc.com.br/rpmfusion/free/el/updates/9/x86_64/
            // Upstream KEY: https://download1.rpmfusion.org/free/el//RPM-GPG-KEY-rpmfusion-free-el-9
            // Mirror URL: https://mirror.versatushpc.com.br/rpmfusion/free/el/RPM-GPG-KEY-rpmfusion-free-el-9
            addRepo(rpmfusionRepos, "rpmfusion",
                    "rpmfusion",
                    EnterpriseLinux::repositoryURL("rpmfusion",
                                                   "free/el/updates/" + releasever + "/" + arch + "/",
                                                   "https://download1.rpmfusion.org/free/el/updates/" + releasever + "/" + arch + "/"),
                    false,
                    EnterpriseLinux::repositoryURL("rpmfusion",
                                                   "free/el/RPM-GPG-KEY-rpmfusion-free-el-" + releasever,
                                                   "https://download1.rpmfusion.org/"));
            addRepoFile(
                baseDir / "rpmfusion.repo", std::move(rpmfusionRepos));
        }

        // epel.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            epelRepos;
            // Upstream URLs:
            // Debug: https://download.fedoraproject.org/pub/epel/8/Everything/x86_64/debug/
            // Source: https://download.fedoraproject.org/pub/epel/8/Everything/source/tree/
            // Main: https://download.fedoraproject.org/pub/epel/8/Everything/x86_64/
            // Note: epel-source and epel-debuginfo has no mirror, the true argument in EnterpriseLinux::repositoryURL
            //   bypass the mirror url
            addRepo(epelRepos, "epel-debuginfo",
                    "Extra Packages for Enterprise Linux " + releasever
                    + " - " + arch + " - Debug",
                    EnterpriseLinux::repositoryURL("epel-debuginfo",
                                  releasever + "/Everything/" + arch + "/debug/",
                                  "https://download.fedoraproject.org/pub/epel/" + releasever + "/Everything/"
                                  + arch + "/debug/", true),
                    false,
                    EnterpriseLinux::repositoryURL("epel-debuginfo",
                                  "RPM-GPG-KEY-EPEL-" + releasever,
                                  "https://download.fedoraproject.org/pub/epel/RPM-GPG-KEY-EPEL-"
                                  + releasever, true));
            addRepo(epelRepos, "epel-source",
                    "Extra Packages for Enterprise Linux " + releasever
                    + " - " + arch + " - Source",
                    EnterpriseLinux::repositoryURL("epel-source",
                                  releasever + "/Everything/source/tree/",
                                  "https://download.fedoraproject.org/pub/epel/" + releasever
                                  + "/Everything/source/tree/", true),
                    false,
                    EnterpriseLinux::repositoryURL("epel-source", "RPM-GPG-KEY-EPEL-" + releasever,
                                  "https://download.fedoraproject.org/pub/epel/RPM-GPG-KEY-EPEL-"
                                  + releasever, true));
            addRepo(epelRepos, "epel",
                    "Extra Packages for Enterprise Linux " + releasever
                    + " - " + arch,
                    EnterpriseLinux::repositoryURL("epel", releasever + "/Everything/" + arch + "/",
                                  "https://download.fedoraproject.org/pub/epel/" + releasever + "/Everything/"
                                  + arch + "/"),
                    false,
                    EnterpriseLinux::repositoryURL("epel", "RPM-GPG-KEY-EPEL-" + releasever,
                                  "https://download.fedoraproject.org/pub/epel/RPM-GPG-KEY-EPEL-"
                                  + releasever));
            addRepoFile(
                baseDir / "epel.repo", std::move(epelRepos));
        }

        // openhpc.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>> openhpcRepos;


            // https://mirror.versatushpc.com.br/openhpc/2/EL_8/x86_64/ohpc-release-2-1.el8.x86_64.rpm
            // https://mirror.versatushpc.com.br/openhpc/3/EL_9/x86_64/ohpc-release-3-1.el9.x86_64.rpm
            // Determine the base URL based on platform
            const std::string ohpcVersion = (releasever == "8") ? "2" : "3";
            const std::string openhpcBaseurl = 
                EnterpriseLinux::repositoryURL("openhpc", ohpcVersion + "/EL_" + releasever + "/",
                              "https://repos.openhpc.community/OpenHPC/" + ohpcVersion + "/EL_" + releasever + "/");
            const std::string openhpcUpdatesBaseurl =
                EnterpriseLinux::repositoryURL("openhpc", ohpcVersion + "/updates/EL_" + releasever + "/",
                              "https://repos.openhpc.community/OpenHPC/" + ohpcVersion + "/updates/EL_" + releasever + "/");

            // Base repository
            addRepo(openhpcRepos, "OpenHPC", "OpenHPC (" + ohpcVersion + ")",
                    openhpcBaseurl,
                    false,
                    "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-OpenHPC-3");

            // Updates repository
            addRepo(openhpcRepos, "OpenHPC-Updates", "OpenHPC Updates (" + ohpcVersion + ")",
                    openhpcUpdatesBaseurl,
                    false,
                    "file:///etc/pki/rpm-gpg/RPM-GPG-KEY-OpenHPC-3");

            addRepoFile(baseDir / "OpenHPC.repo", std::move(openhpcRepos));
        }

        // nvidia.repo
        {
            std::map<std::string, std::shared_ptr<RPMRepository>>
            nvidiaRepos;
            // Upstream URL: https://developer.download.nvidia.com/hpc-sdk/rhel/x86_64/
            addRepo(nvidiaRepos, "nvhpc", "NVIDIA HPC SDK",
                    EnterpriseLinux::repositoryURL("nvhpc", "hpc-sdk/rhel/" + arch,
                                  "https://developer.download.nvidia.com/hpc-sdk/rhel/" + arch, true),
                    false,
                    EnterpriseLinux::repositoryURL("nvhpc", "RPM-GPG-KEY-NVIDIA-HPC-SDK",
                                  "https://developer.download.nvidia.com/hpc-sdk/rhel/"
                                  "RPM-GPG-KEY-NVIDIA-HPC-SDK", true));
            addRepoFile(
                baseDir / "nvidia.repo", std::move(nvidiaRepos));
        }

        return repoFiles;
    }
};

struct RPMDependencyResolver {
    static std::string resolveCodeReadyBuilderName(const OS& osinfo)
    {
        auto distro = osinfo.getDistro();
        auto majorVersion = osinfo.getMajorVersion();
        std::string arch = cloyster::utils::enums::toString(osinfo.getArch());

        switch (distro) {
            case OS::Distro::AlmaLinux:
            case OS::Distro::Rocky:
                return "crb";
            case OS::Distro::RHEL:
                return fmt::format("codeready-builder-for-rhel-{}-{}-rpms",
                    majorVersion, arch);
            case OS::Distro::OL:
                return fmt::format("ol{}_codeready_builder", majorVersion);
            default:
                throw std::runtime_error("Unsupported distro");
        }
    }
};

struct RPMRepositoryGenerator {
    // @FIXME: This will do a lot of HTTP in vain when executed
    //   for the second time. I need a better way to generate these
    //   repositories
    static void generate()
    {
        LOG_DEBUG("Generating the repository files");
        auto repoFiles = ELConfig::getRepositories();
        for (auto& repoFile : repoFiles) {
            try {
                repoFile.save(); // Save each RPMRepositoryFile
            } catch (const std::runtime_error& e) {
                LOG_ERROR("Unexpected error while saving file {} {}", repoFile.path().string(), e.what());
                std::terminate();
            } catch (...) {
                LOG_ERROR("Unexpected unknown error while saving file {}", repoFile.path().string());
                std::terminate();
            }
        }
    }
};

void RepoManager::initializeDefaultRepositories()
{
    auto opts =
        cloyster::Singleton<Options>::get();
    if (opts->dryRun) {
        LOG_WARN("Dry Run: Skipping RepoManager initialization");
        return;
    }
    LOG_INFO("RepoManager initialization");
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            // Generate the repositories files in the disk
            RPMRepositoryGenerator::generate();
            m_impl->rpm.loadBaseDir();
            // enable() is dryRun aware
            enable(RPMDependencyResolver::resolveCodeReadyBuilderName(osinfo));
            break;
        case OS::PackageType::DEB:
            throw std::logic_error("DEB packages not implemented");
            break;
    }
}

void RepoManager::enable(const std::string& repoid)
{
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", repoid);
        return;
    }
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();

    try {
        switch (osinfo.getPackageType()) {
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
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_WARN(
            "Dry Run: Would enable these repos: {}", fmt::join(repos, ","));
        return;
    }
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    try {
        switch (osinfo.getPackageType()) {
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
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", repoid);
        return;
    }

    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    try {
        switch (osinfo.getPackageType()) {
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
    const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
    if (opts->dryRun) {
        LOG_INFO("Dry Run: Would enable repository {}", fmt::join(repos, ","));
        return;
    }

    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    try {
        switch (osinfo.getPackageType()) {
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

    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
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
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            return m_impl->rpm.repos();
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

std::unique_ptr<const IRepository> RepoManager::repo(
    const std::string& repo) const
{
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            return static_cast<std::unique_ptr<const IRepository>>(
                m_impl->rpm.repo(repo));
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

std::vector<std::unique_ptr<const IRepository>> RepoManager::repoFile(
    const std::string& repo) const
{
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            return m_impl->rpm.repoFile(repo);
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

}; // namespace cloyster::services::repos
