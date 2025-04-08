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
#include <vector>

#include <gsl/gsl-lite.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/log.h>
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

    RPMRepositoryFile(std::filesystem::path path, std::unordered_map<std::string, std::shared_ptr<RPMRepository>> repos)
        : m_path(std::move(path)), m_repos(std::move(repos))
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
    std::unordered_map<std::string, std::shared_ptr<RPMRepositoryFile>>
        m_filesIdx;

public:
    static constexpr auto basedir = "/etc/yum.repos.d/";

    // Installs a single .repo file
    void install(const std::filesystem::path& source)
    {
        const auto& dest = basedir / source.filename();
        const auto opts = cloyster::Singleton<cloyster::services::Options>::get();

        // Do not copy the file to the basedir if it
        // is already there
        if (source != dest) {
            cloyster::copyFile(source, dest);
        }

        if (opts->dryRun) {
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
    void loadDir(const std::filesystem::path& path)
    {
        const auto opts = cloyster::Singleton<cloyster::services::Options>::get();
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
                = fmt::format("Cannot enable repository {}, no such repository "
                              "loaded, repositories: available: {}",
                    repoName, fmt::join(repos, ","));
            throw std::runtime_error(msg);
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

    static std::vector<RPMRepositoryFile> getRepositories() {
        const auto& osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
        std::string releasever = std::to_string(osinfo.getMajorVersion());            // e.g., "8"
        std::string arch = cloyster::utils::enums::toString(osinfo.getArch());        // e.g., "x86_64"
        const auto opts = cloyster::Singleton<cloyster::services::Options>::get();

        // Local function for air-gap URL construction
        auto makeAirGapUrl = [&](const std::string& repoName, const std::string& path, const std::string& originalUrl) {
            return opts->airGap ? opts->airGapUrl + "/" + repoName + "/" + path : originalUrl;
        };

        std::vector<RPMRepositoryFile> repoFiles;

        auto addRepo = [](std::unordered_map<std::string, std::shared_ptr<RPMRepository>>& repoMap,
                          // NOLINTNEXTLINE
                          const std::string& id, const std::string& name, 
                          const std::string& baseurl, bool enabled, 
                          const std::string& gpgkey) {
            auto repo = std::make_shared<RPMRepository>();
            repo->id(id);
            repo->name(name);
            repo->baseurl(baseurl);
            repo->enabled(enabled);
            repo->gpgcheck(true);
            repo->gpgkey(gpgkey);
            repoMap.emplace(id, repo);
        };

        // oracle.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> oracleRepos;
            addRepo(oracleRepos, "OLBaseOS", "Oracle Linux " + releasever + " BaseOS Latest ($basearch)",
                    makeAirGapUrl("OLBaseOS", "repo/OracleLinux/OL" + releasever + "/baseos/latest/$basearch/",
                                  "https://yum$ociregion.oracle.com/repo/OracleLinux/OL" + releasever + "/baseos/latest/$basearch/"),
                    false,
                    makeAirGapUrl("OLBaseOS", "RPM-GPG-KEY-oracle-ol" + releasever,
                                  "https://yum.oracle.com/RPM-GPG-KEY-oracle-ol" + releasever));
            repoFiles.emplace_back(baseDir / "oracle.repo", std::move(oracleRepos));
        }

        // rocky.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> rockyRepos;
            addRepo(rockyRepos, "RockyBaseOS", "Rocky Linux $releasever - BaseOS",
                    makeAirGapUrl("RockyBaseOS", "$contentdir/" + releasever + "/BaseOS/$basearch/os/",
                                  "http://dl.rockylinux.org/$contentdir/" + releasever + "/BaseOS/$basearch/os/"),
                    false,
                    makeAirGapUrl("RockyBaseOS", "RPM-GPG-KEY-Rocky-" + releasever,
                                  opts->mirrorBaseUrl + "/rocky/linux/RPM-GPG-KEY-Rocky-" + releasever));
            repoFiles.emplace_back(baseDir / "rocky.repo", std::move(rockyRepos));
        }

        // almalinux.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> almaRepos;
            addRepo(almaRepos, "AlmaLinuxBaseOS", "AlmaLinux $releasever - BaseOS",
                    makeAirGapUrl("AlmaLinuxBaseOS", "almalinux/" + releasever + "/BaseOS/$basearch/os/",
                                  "https://repo.almalinux.org/almalinux/" + releasever + "/BaseOS/$basearch/os/"),
                    false,
                    makeAirGapUrl("AlmaLinuxBaseOS", "RPM-GPG-KEY-AlmaLinux-" + releasever,
                                  opts->mirrorBaseUrl + "/almalinux/almalinux/RPM-GPG-KEY-AlmaLinux-" + releasever));
            repoFiles.emplace_back(baseDir / "almalinux.repo", std::move(almaRepos));
        }

        // beegfs.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> beegfsRepos;
            addRepo(beegfsRepos, "beegfs", "BeeGFS",
                    makeAirGapUrl("beegfs", opts->beegfsVersion + "/dists/rhel" + releasever + "/",
                                  opts->mirrorBaseUrl + "/" + opts->beegfsVersion + "/dists/rhel" + releasever + "/"),
                    false,
                    makeAirGapUrl("beegfs", opts->beegfsVersion + "/gpg/GPG-KEY-beegfs",
                                  opts->mirrorBaseUrl + "/" + opts->beegfsVersion + "/gpg/GPG-KEY-beegfs"));
            repoFiles.emplace_back(baseDir / "beegfs.repo", std::move(beegfsRepos));
        }

        // grafana.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> grafanaRepos;
            addRepo(grafanaRepos, "grafana", "grafana",
                    makeAirGapUrl("grafana", "", opts->mirrorBaseUrl + "/grafana/"),
                    false,
                    makeAirGapUrl("grafana", "gpg.key", "https://rpm.grafana.com/gpg.key"));
            repoFiles.emplace_back(baseDir / "grafana.repo", std::move(grafanaRepos));
        }

        // influxdata.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> influxRepos;
            addRepo(influxRepos, "influxdata", "InfluxData Repository - Stable",
                    makeAirGapUrl("influxdata", "", opts->mirrorBaseUrl + "/influxdata/"),
                    false,
                    makeAirGapUrl("influxdata", "influxdata-archive_compat.key",
                                  "https://repos.influxdata.com/influxdata-archive_compat.key"));
            repoFiles.emplace_back(baseDir / "influxdata.repo", std::move(influxRepos));
        }

        // intel.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> intelRepos;
            addRepo(intelRepos, "oneAPI", "Intel oneAPI repository",
                    makeAirGapUrl("oneAPI", "", opts->mirrorBaseUrl + "/oneAPI/"),
                    false,
                    makeAirGapUrl("oneAPI", "GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB",
                                  "https://yum.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2019.PUB"));
            repoFiles.emplace_back(baseDir / "intel.repo", std::move(intelRepos));
        }

        // zabbix.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> zabbixRepos;
            addRepo(zabbixRepos, "zabbix", "zabbix",
                    makeAirGapUrl("zabbix", "zabbix/6.4/rhel/" + releasever + "/" + arch + "/",
                                  opts->mirrorBaseUrl + "/zabbix/zabbix/6.4/rhel/" + releasever + "/" + arch + "/"),
                    false,
                    makeAirGapUrl("zabbix", "RPM-GPG-KEY-ZABBIX",
                                  opts->mirrorBaseUrl + "/zabbix/RPM-GPG-KEY-ZABBIX"));
            repoFiles.emplace_back(baseDir / "zabbix.repo", std::move(zabbixRepos));
        }

        // elrepo.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> elrepoRepos;
            addRepo(elrepoRepos, "elrepo", "elrepo",
                    makeAirGapUrl("elrepo", "elrepo/el" + releasever + "/" + arch + "/",
                                  opts->mirrorBaseUrl + "/elrepo/elrepo/el" + releasever + "/" + arch + "/"),
                    false,
                    makeAirGapUrl("elrepo", "RPM-GPG-KEY-elrepo.org",
                                  "https://www.elrepo.org/RPM-GPG-KEY-elrepo.org"));
            repoFiles.emplace_back(baseDir / "elrepo.repo", std::move(elrepoRepos));
        }

        // rpmfusion.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> rpmfusionRepos;
            addRepo(rpmfusionRepos, "rpmfusion-free-updates", "rpmfusion-free-updates",
                    makeAirGapUrl("rpmfusion-free-updates", "free/el/updates/" + releasever + "/" + arch + "/",
                                  opts->mirrorBaseUrl + "/rpmfusion/free/el/updates/" + releasever + "/" + arch + "/"),
                    false,
                    makeAirGapUrl("rpmfusion-free-updates", "RPM-GPG-KEY-rpmfusion-free-el-" + releasever,
                                  opts->mirrorBaseUrl + "/rpmfusion/free/el/RPM-GPG-KEY-rpmfusion-free-el-" + releasever));
            repoFiles.emplace_back(baseDir / "rpmfusion.repo", std::move(rpmfusionRepos));
        }

        // epel.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> epelRepos;
            addRepo(epelRepos, "epel", "Extra Packages for Enterprise Linux " + releasever + " - $basearch",
                    makeAirGapUrl("epel", releasever + "/Everything/" + arch + "/",
                                  opts->mirrorBaseUrl + "/epel/" + releasever + "/Everything/" + arch + "/"),
                    false,
                    makeAirGapUrl("epel", "RPM-GPG-KEY-EPEL-" + releasever,
                                  opts->mirrorBaseUrl + "/epel/RPM-GPG-KEY-EPEL-" + releasever));
            addRepo(epelRepos, "epel-debuginfo", "Extra Packages for Enterprise Linux " + releasever + " - $basearch - Debug",
                    makeAirGapUrl("epel-debuginfo", releasever + "/Everything/" + arch + "/debug/",
                                  opts->mirrorBaseUrl + "/epel/" + releasever + "/Everything/" + arch + "/debug/"),
                    false,
                    makeAirGapUrl("epel-debuginfo", "RPM-GPG-KEY-EPEL-" + releasever,
                                  opts->mirrorBaseUrl + "/epel/RPM-GPG-KEY-EPEL-" + releasever));
            addRepo(epelRepos, "epel-source", "Extra Packages for Enterprise Linux " + releasever + " - $basearch - Source",
                    makeAirGapUrl("epel-source", releasever + "/Everything/source/tree/",
                                  opts->mirrorBaseUrl + "/epel/" + releasever + "/Everything/source/tree/"),
                    false,
                    makeAirGapUrl("epel-source", "RPM-GPG-KEY-EPEL-" + releasever,
                                  opts->mirrorBaseUrl + "/epel/RPM-GPG-KEY-EPEL-" + releasever));
            repoFiles.emplace_back(baseDir / "epel.repo", std::move(epelRepos));
        }

        // openhpc.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> openhpcRepos;
            addRepo(openhpcRepos, "openhpc", "OpenHPC",
                    makeAirGapUrl("openhpc", "2/EL_" + releasever + "/",
                                  opts->mirrorBaseUrl + "/openhpc/2/EL_" + releasever + "/"),
                    false,
                    makeAirGapUrl("openhpc", "public_key",
                                  "https://obs.openhpc.community/projects/OpenHPC/public_key"));
            addRepo(openhpcRepos, "openhpc-updates", "OpenHPC Updates",
                    makeAirGapUrl("openhpc-updates", "2/updates/EL_" + releasever + "/",
                                  opts->mirrorBaseUrl + "/openhpc/2/updates/EL_" + releasever + "/"),
                    false,
                    makeAirGapUrl("openhpc-updates", "public_key",
                                  "https://obs.openhpc.community/projects/OpenHPC/public_key"));
            repoFiles.emplace_back(baseDir / "openhpc.repo", std::move(openhpcRepos));
        }

        // nvidia.repo
        {
            std::unordered_map<std::string, std::shared_ptr<RPMRepository>> nvidiaRepos;
            addRepo(nvidiaRepos, "nvhpc", "NVIDIA HPC SDK",
                    makeAirGapUrl("nvhpc", "hpc-sdk/rhel/$basearch",
                                  "https://developer.download.nvidia.com/hpc-sdk/rhel/$basearch"),
                    false,
                    makeAirGapUrl("nvhpc", "RPM-GPG-KEY-NVIDIA-HPC-SDK",
                                  "https://developer.download.nvidia.com/hpc-sdk/rhel/RPM-GPG-KEY-NVIDIA-HPC-SDK"));
            repoFiles.emplace_back(baseDir / "nvidia.repo", std::move(nvidiaRepos));
        }

        return repoFiles;
    }
};

struct RPMDependencyResolver {
    static std::string resolveCodeReadyBuilderName(const OS& osinfo) {
        auto distro = osinfo.getDistro();
        auto majorVersion = osinfo.getMajorVersion();
        std::string arch = cloyster::utils::enums::toString(osinfo.getArch());

        switch (distro) {
            case OS::Distro::AlmaLinux:
            case OS::Distro::Rocky:
                return "crb";
            case OS::Distro::RHEL:
                return fmt::format("codeready-builder-for-rhel-{}-{}-rpms", majorVersion, arch);
            case OS::Distro::OL:
                return fmt::format("ol{}_codeready_builder", majorVersion);
            default:
                throw std::runtime_error("Unsupported distro");
        }
    }
};

struct RPMRepositoryGenerator {
    static void generate() {
        auto repoFiles = ELConfig::getRepositories();
        for (auto& repoFile : repoFiles) {
            repoFile.save(); // Save each RPMRepositoryFile
        }
    }
};

void RepoManager::initializeDefaultRepositories()
{
    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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
    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();

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
    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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

    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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

    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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

    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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
    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
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
    auto osinfo = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            return static_cast<std::unique_ptr<const IRepository>>(
                m_impl->rpm.repo(repo));
            break;
        default:
            throw std::logic_error("Not implemented");
    }
}

}; // namespace cloyster::services::repos
