/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <gsl/gsl-lite.hpp>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/patterns/wrapper.h>
#include <cloysterhpc/models/cluster.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/init.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/options.h>
#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/services/repos.h>
#include <cloysterhpc/services/runner.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

using cloyster::concepts::IsParser;
using cloyster::services::files::IsKeyFileReadable;
using cloyster::services::files::KeyFile;
using cloyster::services::repos::IRepository;
using std::filesystem::path;

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
                cloyster::functions::abort(
                    "Could not load repo name from repo '{}'", repogroup);
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
        const std::map<std::string, std::shared_ptr<RPMRepository>>& repos,
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
    std::filesystem::path m_path;

    // @FIXME: Double check if this is required to be shared_ptr
    std::map<std::string, std::shared_ptr<RPMRepository>> m_repos;

public:
    explicit RPMRepositoryFile(auto path)
        : m_path(std::move(path))
    {
        RPMRepositoryParser::parse(m_path, m_repos);
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

    void save() const
    {
        LOG_DEBUG("Saving {}", m_path.string());
        RPMRepositoryParser::unparse(m_repos, m_path);
    }
};

TEST_SUITE_BEGIN("cloyster::services::repos");

/**
 * @brief Decouples filesystem and network I/O from the MirrorRepoConfig
 *   and UpstreamRepoConfig
 */
struct DefaultMirrorExistenceChecker final {
    [[nodiscard]] static bool pathExists(const std::filesystem::path& path)
    {
        return cloyster::functions::exists(path);
    }

    [[nodiscard]] static bool urlExists(const std::string& url)
    {
        return cloyster::functions::getHttpStatus(url) == "200";
    }
};

// For testing
struct FalseMirrorExistenceChecker final {
    [[nodiscard]] static bool pathExists(const std::filesystem::path& path)
    {
        static_cast<void>(path);
        return false;
    }

    [[nodiscard]] static bool urlExists(const std::string& url)
    {
        static_cast<void>(url);
        return false;
    }
};

// For testing
struct TrueMirrorExistenceChecker final {
    [[nodiscard]] static bool pathExists(const std::filesystem::path& path)
    {
        static_cast<void>(path);
        return true;
    }

    [[nodiscard]] static bool urlExists(const std::string& url)
    {
        static_cast<void>(url);
        return true;
    }
};

// Represents repository path/url and gpg path/url
struct RepoPaths final {
    std::string repo;
    std::optional<std::string> gpgkey = std::nullopt;
};

// Uniquely identify a remote repository
struct RepoId final {
    std::string id;
    std::string name;
    std::string filename;
};

// Upstream and Mirror configuration for a single repository
struct RepoConfig final {
    RepoId repoId;
    RepoPaths mirror;
    RepoPaths upstream;
};

// Represent variables values present in repos.conf to be interpolated during
// the parsing
struct RepoConfigVars final {
    std::string arch; // ex: x86_64
    std::string beegfsVersion; // beegfs_<version>, ex: beegfs_7.3.3
    std::string ohpcVersion; // major, ex: 3
    std::string osversion; // major.minor, ex: 9.5
    std::string releasever; // major, ex: 9
    std::string xcatVersion; // major.minor, ex: 2.17 or latest
    std::string zabbixVersion; // major.minor, ex: 6.4
};

// Represents a Mirror Repository
template <typename MirrorExistenceChecker = DefaultMirrorExistenceChecker>
struct MirrorRepo final {
    RepoPaths paths;

    [[nodiscard]] std::string baseurl() const
    {
        const auto opts = cloyster::Singleton<Options>::get();

        return cloyster::utils::string::rstrip(fmt::format("{mirrorUrl}/{path}",
            fmt::arg("mirrorUrl", opts->mirrorBaseUrl),
            fmt::arg("path", paths.repo)), "/");
    };

    [[nodiscard]] std::optional<std::string> gpgkey() const
    {
        if (!paths.gpgkey) {
            return std::nullopt;
        }

        const auto opts = cloyster::Singleton<Options>::get();
        return fmt::format("{mirrorUrl}/{path}",
            fmt::arg("mirrorUrl", opts->mirrorBaseUrl),
            fmt::arg("path", paths.gpgkey.value()));
    }

    [[nodiscard]] static bool isLocalUrl(const std::string_view url)
    {
        return url.starts_with("file://");
    }

    [[nodiscard]] static std::string localPath(const std::string_view url)
    {
        assert(isLocalUrl(url));
        return std::string(url.substr(std::string_view("file://").length()));
    }

    [[nodiscard]] bool exists() const
    {
        if (paths.repo.empty()) {
            return false;
        }
        const auto opts = cloyster::Singleton<Options>::get();
        if (isLocalUrl(opts->mirrorBaseUrl)) {
            return MirrorExistenceChecker::pathExists(localPath(baseurl()));
        } else {
            return MirrorExistenceChecker::urlExists(baseurl() + "/repodata/repomd.xml");
        }
    }
};

TEST_CASE("MirrorRepo")
{
    // NOLINTNEXTLINE
    auto opts = Options { .mirrorBaseUrl = "https://mirror.example.com" };
    cloyster::Singleton<Options>::init(std::make_unique<Options>(opts));
    // Log::init(5);

    auto mirrorConfigOnline = MirrorRepo<TrueMirrorExistenceChecker> { .paths
        = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    auto mirrorConfigOffline = MirrorRepo<FalseMirrorExistenceChecker> { .paths
        = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    CHECK(mirrorConfigOnline.baseurl()
        == "https://mirror.example.com/myrepo/repo");
    CHECK(mirrorConfigOnline.gpgkey().value()
        == "https://mirror.example.com/myrepo/key.gpg");

    // Test local paths
    cloyster::Singleton<Options>::get()->mirrorBaseUrl
        = "file:///var/run/repos";
    CHECK(mirrorConfigOnline.baseurl() == "file:///var/run/repos/myrepo/repo");
    CHECK(mirrorConfigOnline.gpgkey().value()
        == "file:///var/run/repos/myrepo/key.gpg");

    // Test existence
    CHECK(mirrorConfigOnline.exists());
    CHECK(!mirrorConfigOffline.exists());
}

// Represents an upstream repository
template <typename MirrorExistenceChecker = DefaultMirrorExistenceChecker>
struct UpstreamRepo final {
    RepoPaths paths;

    [[nodiscard]] std::string baseurl() const
    {
        return cloyster::utils::string::rstrip(paths.repo, "/");
    };

    [[nodiscard]] std::optional<std::string> gpgurl() const
    {
        if (!paths.gpgkey) {
            return std::nullopt;
        }

        return paths.gpgkey;
    };

    [[nodiscard]] constexpr bool exists() const
    {
        return MirrorExistenceChecker::urlExists(baseurl() + "/repodata/repomd.xml");
    };

    [[nodiscard]] constexpr std::optional<std::string> gpgkey() const
    {
        if (!paths.gpgkey) {
            return std::nullopt;
        }

        const auto url = gpgurl();
        if (!url) {
            return std::nullopt;
        }

        if (MirrorExistenceChecker::urlExists(url.value())) {
            return gpgurl();
        } else {
            LOG_WARN(
                "GPG not found, assuming disabled GPG check {}", url.value());
            return std::nullopt;
        }
    }
};

// Chose between upstream or mirror repository
struct RepoChooser final {
    enum class Choice : bool { UPSTREAM, MIRROR };

    template <typename MirrorChecker, typename UpstreamChecker>
    static constexpr Choice choose(const MirrorRepo<MirrorChecker>& mirror,
        const UpstreamRepo<UpstreamChecker>& upstream,
        const bool forceUpstream = false)
    {
        if (forceUpstream) {
            return Choice::UPSTREAM;
        }

        const auto opts = cloyster::Singleton<Options>::get();
        if (opts->disableMirrors) {
            return Choice::UPSTREAM;
        }

        if (!mirror.exists()) {
            LOG_WARN("Mirror does not exists falling back to upstream {}",
                upstream.baseurl());

            if (!upstream.exists()) {
                LOG_WARN("Upstream URL error, is the URL correct? {}",
                    upstream.baseurl());
            }
            return Choice::UPSTREAM;
        }

        return Choice::MIRROR;
    }
};

TEST_CASE("RepoChooser")
{
    // NOLINTNEXTLINE
    auto opts = Options { .mirrorBaseUrl = "https://mirror.example.com" };
    cloyster::Singleton<Options>::init(std::make_unique<Options>(opts));
    // Log::init(5);

    auto mirrorConfigOnline
        = MirrorRepo<TrueMirrorExistenceChecker> { .paths
              = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    auto mirrorConfigOffline
        = MirrorRepo<FalseMirrorExistenceChecker> { .paths
              = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    auto upstreamConfig = UpstreamRepo<TrueMirrorExistenceChecker> { .paths
        = { .repo = "https://upstream.example.com/upstream/repo",
            .gpgkey = "https://upstream.example.com/upstream/key.gpg" } };

    cloyster::Singleton<Options>::get()->disableMirrors = false;
    auto choice1 = RepoChooser::choose(mirrorConfigOnline, upstreamConfig);
    CHECK(choice1 == RepoChooser::Choice::MIRROR);
    auto choice2
        = RepoChooser::choose(mirrorConfigOffline, upstreamConfig);
    CHECK(choice2 == RepoChooser::Choice::UPSTREAM);
}

// Converts RepoConfig to RPMRepository do HTTP requests to decide
// between upstream or mirror
struct RepoAssembler final {
    template <typename MChecker, typename UChecker>
    static constexpr RPMRepository assemble(const RepoId& repoid,
        const MirrorRepo<MChecker>& mirror,
        const UpstreamRepo<UChecker>& upstream, const bool enabled = false,
        const bool forceUpstream = false)
    {
        auto repo = RPMRepository {};
        repo.group(static_cast<std::string>(repoid.id));
        repo.id(static_cast<std::string>(repoid.id));
        repo.name(static_cast<std::string>(repoid.name));
        repo.source(repoid.filename);
        repo.enabled(enabled);

        const auto choice
            = RepoChooser::choose(mirror, upstream, forceUpstream);
        switch (choice) {
            case RepoChooser::Choice::UPSTREAM:
                repo.baseurl(upstream.baseurl());
                repo.gpgkey(upstream.gpgkey());
                repo.gpgcheck(upstream.gpgkey().has_value());
                break;
            case RepoChooser::Choice::MIRROR:
                repo.baseurl(mirror.baseurl());
                repo.gpgkey(mirror.gpgkey());
                repo.gpgcheck(mirror.gpgkey().has_value());
                break;
        }

        return repo;
    }
};

TEST_CASE("RepoAssembler")
{
    // NOLINTNEXTLINE
    auto opts = Options { .mirrorBaseUrl = "https://mirror.example.com" };
    cloyster::Singleton<Options>::init(std::make_unique<Options>(opts));
    // Log::init(5);

    auto mirrorConfigOffline
        = MirrorRepo<FalseMirrorExistenceChecker> { .paths
              = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    auto mirrorConfigOnline
        = MirrorRepo<TrueMirrorExistenceChecker> { .paths
              = { .repo = "myrepo/repo", .gpgkey = "myrepo/key.gpg" } };
    auto upstreamConfig = UpstreamRepo<TrueMirrorExistenceChecker> { .paths
        = { .repo = "https://upstream.example.com/upstream/repo",
            .gpgkey = "https://upstream.example.com/upstream/key.gpg" } };

    // If mirror is offline it should fallback to upstream
    CHECK(RepoChooser::choose(mirrorConfigOffline, upstreamConfig)
        == RepoChooser::Choice::UPSTREAM);
    // If mirror is online it should chose the mirror
    CHECK(RepoChooser::choose(mirrorConfigOnline, upstreamConfig)
        == RepoChooser::Choice::MIRROR);

    // Disable mirrors
    cloyster::Singleton<Options>::get()->disableMirrors = true;

    // If mirrors are disabled it should choose the upstream even if the
    // mirror is online
    CHECK(RepoChooser::choose(mirrorConfigOnline, upstreamConfig)
        == RepoChooser::Choice::UPSTREAM);

    auto repoId = RepoId {
        .id = "myrepo",
        .name = "My very cool repository full of packages",
        .filename = "myrepo.repo",
    };

    auto repoUpstream = RepoAssembler::assemble(
        repoId, mirrorConfigOffline, upstreamConfig);
    CHECK(repoUpstream.baseurl().value() == upstreamConfig.baseurl());

    // Enable mirrors again
    cloyster::Singleton<Options>::get()->disableMirrors = false;
    auto repoMirror = RepoAssembler::assemble(
        repoId, mirrorConfigOnline, upstreamConfig);
    // CHECK(repoMirror.baseurl().value() == mirrorConfigOnline.baseurl());
}

// In-memory representation of repos.conf
class RepoConfFile final {
    // RepoConfigs grouped by file name
    std::map<std::string, std::vector<RepoConfig>> m_files;

public:
    void insert(const std::string& filename, RepoConfig& value)
    {
        if (!m_files.contains(filename)) {
            m_files.emplace(filename, std::vector<RepoConfig> {});
        }
        m_files.at(filename).push_back(value);
    }

    [[nodiscard]] const auto& at(const std::string& key) const
    {
        return m_files.at(key);
    }

    // RepoConfigs grouped by file name
    [[nodiscard]] const auto& files() const { return m_files; }
    //
    // RepoConfigs grouped by file name
    [[nodiscard]] std::vector<std::string> filesnames() const
    {
        return m_files
            | std::views::transform([](const auto& pair) { return pair.first; })
            | std::ranges::to<std::vector>();
    }

    // Find a RepoConfig by repository id, if it exists
    [[nodiscard]] std::optional<RepoConfig> find(const auto& repoid) const
    {
        for (const auto& [_, configs] : m_files) {
            for (const auto& config : configs) {
                if (config.repoId.id == repoid) {
                    return config;
                }
            }
        };

        return std::nullopt;
    }
};

// Parser for repos.conf
class RepoConfigParser final {
    static std::string interpolateVars(const auto& fmt, const auto& vars)
    {
        return fmt::format(fmt::runtime(fmt),
            fmt::arg("releasever", vars.releasever),
            fmt::arg("osversion", vars.osversion), fmt::arg("arch", vars.arch),
            fmt::arg("beegfsVersion", vars.beegfsVersion),
            fmt::arg("zabbixVersion", vars.zabbixVersion),
            fmt::arg("xcatVersion", vars.xcatVersion),
            fmt::arg("ohpcVersion", vars.ohpcVersion));
    };

public:
    static constexpr std::string_view defaultPath = "/opt/cloysterhpc/conf/repos.conf";
    static void parse(const std::filesystem::path& path, RepoConfFile& output,
        const RepoConfigVars& vars)
    {
        if (!cloyster::functions::exists(path)) {
            cloyster::functions::abort("Trying to parse {} but it does not exists at {}", path);
        }
        auto file = KeyFile(path);
        auto repoNames = file.getGroups();

        for (const auto& repoGroup : repoNames) {
            RepoConfig repo;

            // repoId.id (no placeholders)
            repo.repoId.id = repoGroup;

            // name
            auto name = file.getString(repoGroup, "name");
            if (name.empty()) {
                cloyster::functions::abort(
                    "Could not load name from repo '{}'", repoGroup);
            }
            try {
                repo.repoId.name = interpolateVars(name, vars);
            } catch (const fmt::format_error& e) {
                cloyster::functions::abort(
                    "Failed to format name for repo '{}': {}", repoGroup,
                    e.what());
            }

            // filename (no placeholders)
            repo.repoId.filename = file.getString(repoGroup, "filename");
            if (repo.repoId.filename.empty()) {
                cloyster::functions::abort(
                    "Could not load filename from repo '{}'", repoGroup);
            }

            // mirror.repo
            const auto mirrorRepoOpt = file.getStringOpt(repoGroup, "mirror.repo");
            if (mirrorRepoOpt) {
                const auto mirrorRepo = mirrorRepoOpt.value();
                try {
                    repo.mirror.repo = interpolateVars(mirrorRepo, vars);
                } catch (const fmt::format_error& e) {
                    cloyster::functions::abort(
                        "Could not interpolate mirror.repo from repo '{}'", repoGroup);
                }
            } else {
                repo.mirror.repo = "";
            }

            // mirror.gpgkey (optional)
            auto mirrorGpgkey = file.getStringOpt(repoGroup, "mirror.gpgkey");
            if (mirrorGpgkey) {
                try {
                    repo.mirror.gpgkey
                        = interpolateVars(mirrorGpgkey.value(), vars);
                } catch (const fmt::format_error& e) {
                    cloyster::functions::abort(
                        "Failed to format mirror.gpgkey for repo '{}': {}",
                        repoGroup, e.what());
                }
            } else {
                repo.mirror.gpgkey = std::nullopt;
            }

            // upstream.repo
            auto upstreamRepo = file.getString(repoGroup, "upstream.repo");
            if (upstreamRepo.empty()) {
                cloyster::functions::abort(
                    "Could not load upstream.repo from repo '{}'", repoGroup);
            }
            try {
                repo.upstream.repo = interpolateVars(upstreamRepo, vars);
            } catch (const fmt::format_error& e) {
                cloyster::functions::abort(
                    "Failed to format upstream.repo for repo '{}': {}",
                    repoGroup, e.what());
            }

            // upstream.gpgkey (optional)
            auto upstreamGpgkey
                = file.getStringOpt(repoGroup, "upstream.gpgkey");
            if (upstreamGpgkey) {
                try {
                    repo.upstream.gpgkey
                        = interpolateVars(upstreamGpgkey.value(), vars);
                } catch (const fmt::format_error& e) {
                    cloyster::functions::abort(
                        "Failed to format upstream.gpgkey for repo '{}': {}",
                        repoGroup, e.what());
                }
            } else {
                repo.upstream.gpgkey = std::nullopt;
            }

            LOG_TRACE("Loaded repository configuration for {} from {}", interpolateVars(repo.repoId.id, vars), path);
            output.insert(repo.repoId.filename, repo);
        }
    };

    // Parse a repo.conf file and return a RepoConfFile with default vars,
    // for testing only
    static RepoConfFile parseTest(
        const std::filesystem::path& path,
        const RepoConfigVars& vars = RepoConfigVars {
            .arch = "x86_64",
            .beegfsVersion = "beegfs_7.3.3",
            .ohpcVersion = "3",
            .osversion = "9.4",
            .releasever = "9",
            .xcatVersion = "latest",
            .zabbixVersion = "6.4",
        }
    )
    {
        RepoConfFile conffile;
        parse(path, conffile, vars);
        return conffile;
    };

    // Parse a repo.conf file and return a RepoConfFile using default path
    static RepoConfFile parse(
        const RepoConfigVars& vars)
    {
        RepoConfFile conffile;
        parse(defaultPath, conffile, vars);
        return conffile;
    };
};

TEST_CASE("RepoConfigParser")
{
    REQUIRE(cloyster::functions::exists("repos/repos.conf"));
    auto conffile = RepoConfigParser::parseTest("repos/repos.conf");
    CHECK(conffile.files().size() > 0);
    CHECK(conffile.files().contains("epel.repo"));

    const auto epelOpt = conffile.find("epel");
    CHECK(epelOpt.has_value() == true);
    const auto& epel = epelOpt.value();
    CHECK(epel.mirror.repo == "epel/9/Everything/x86_64/");
    CHECK(epel.upstream.repo
        == "https://download.fedoraproject.org/pub/epel/9/Everything/"
           "x86_64/");
}

// Installs and enable/disable RPM repositories
class RPMRepoManager final {
    static constexpr auto m_parser = RPMRepositoryParser();
    // Maps repo id to files
    std::map<std::string, std::shared_ptr<RPMRepositoryFile>> m_filesIdx;

public:
    static constexpr std::string_view basedir = "/etc/yum.repos.d/";

    // Installs a single .repo file
    void install(const std::filesystem::path& source)
    {
        const auto& dest = basedir / source.filename();
        const auto opts
            = cloyster::Singleton<cloyster::services::Options>::get();

        // Do not copy the file to the basedir if it
        // is already there
        if (source != dest) {
            cloyster::functions::copyFile(source, dest);
        }

        if (opts->dryRun) {
            LOG_INFO("Dry Run: Would open {}", dest.string());
            return;
        }

        const auto& repofile
            = std::make_shared<RPMRepositoryFile>(RPMRepositoryFile(dest));
        LOG_ASSERT(repofile->repos().size() > 0, "BUG Loading file");
        for (auto& [repo, _] : repofile->repos()) {
            LOG_TRACE("{} loaded", repo);
            m_filesIdx.emplace(repo, repofile);
        }
    }

    // Install all .repo files inside a folder
    void install(std::filesystem::directory_iterator&& dirIter)
    {
        for (const auto& fil : std::move(dirIter)) {
            std::string fname = fil.path().filename().string();
            if (fname.ends_with(".repo")) {
                LOG_TRACE("Loading {}", fname);
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

    static std::vector<std::unique_ptr<const IRepository>> repoFile(
        const std::string& repoFileName)
    {
        try {
            auto path = fmt::format("{}/{}", basedir, repoFileName);
            auto repos = RPMRepositoryFile(path).repos()
                // We copy to cons unique to express that these values cannot
                // be changed through this API
                | std::views::transform([](auto&& pair) {
                      return std::make_unique<const RPMRepository>(
                          *pair.second);
                  })
                | std::ranges::to<
                    std::vector<std::unique_ptr<const IRepository>>>();
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
        LOG_DEBUG("{} RPM repo {}", value ? "Enabling" : "Disabling", repo);
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
            try {
                auto& rfile = m_filesIdx.at(repo);
                toSave.emplace(rfile);
                enable(repo, rfile, value);
            } catch (const std::out_of_range&) {
                cloyster::functions::abort("Trying to enable unknown repository {}, "
                          "failed because the repository was not found.",
                    repo);
            }
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

// Adpater for simplifying the conversion from RepoConfig
// to RPMRepository and RPMRepositoryFiles, may do HTTP requests
//
template <typename MChecker = DefaultMirrorExistenceChecker,
    typename UChecker = DefaultMirrorExistenceChecker>
struct RepoConfAdapter final {
    static RPMRepository fromConfig(const RepoConfig& config)
    {
        const RepoId& repoid = config.repoId;
        const MirrorRepo<MChecker> mirror {
            .paths = config.mirror,
        };
        const UpstreamRepo<UChecker> upstream {
            .paths = config.upstream,
        };
        return RepoAssembler::assemble<MChecker, UChecker>(
            repoid, mirror, upstream);
    };

    static RPMRepositoryFile fromConfigs(const std::string& filename,
        const std::vector<RepoConfig>& configs,
        const std::filesystem::path& basedir = RPMRepoManager::basedir)
    {
        const auto path = basedir / filename;
        std::map<std::string, std::shared_ptr<RPMRepository>> repos;
        for (const auto& config : configs) {
            repos.emplace(config.repoId.id,
                std::make_shared<RPMRepository>(fromConfig(config)));
        };
        return { path, repos };
    }

    // Convert RepoConfFile to a list of RPMRepository files using
    // the repoList
    static std::vector<RPMRepositoryFile> fromConfFile(
        const RepoConfFile& conffile, const std::vector<std::string>& repoList,
        const std::filesystem::path& basedir = RPMRepoManager::basedir)
    {
        std::vector<RPMRepositoryFile> output;
        for (const auto& [filename, configs] : conffile.files()) {
            if (!cloyster::functions::isIn(repoList, filename)) {
                continue;
            }

            LOG_INFO("Generating {}", filename);

            auto&& repofile = fromConfigs(filename, configs, basedir);
            output.push_back(std::move(repofile));
        }
        return output;
    }
};

TEST_CASE("RepoAdapter")
{
    Options opts {};
    cloyster::services::initializeSingletonsOptions(
        std::make_unique<Options>(opts));
    // Log::init(5);

    const auto conffile = RepoConfigParser::parseTest("repos/repos.conf");
    const auto repofiles = RepoConfAdapter<TrueMirrorExistenceChecker,
        TrueMirrorExistenceChecker>::fromConfFile(conffile,
        conffile.filesnames());
    CHECK(repofiles.size() == conffile.files().size());
};

// Filter repositories based on the distribution and non-distro repos
template <typename ShouldUseVaultService = RockyLinux>
class RepoFilter final {
    std::vector<std::string> m_allRepoFilesNames;
    OS m_osinfo;

public:
    static constexpr auto distroFilesNames = {
        "rhel.repo",
        "rocky.repo",
        "rocky-vault.repo",
        "oracle.repo",
        "almalinux.repo",
    };

    RepoFilter(const RepoConfFile& conffile, const OS& osinfo)
        : m_allRepoFilesNames(conffile.filesnames())
        , m_osinfo(osinfo)
    {}

    [[nodiscard]] std::vector<std::string> distroRepos() const
    {
        switch (m_osinfo.getDistro()) {
            case OS::Distro::OL:
                return filterOracleLinux();
            case OS::Distro::RHEL:
                return filterRHEL();
            case OS::Distro::AlmaLinux:
                return filterAlmaLinux();
            case OS::Distro::Rocky:
                return filterRockyLinux();
        };

        std::unreachable();
    }

    [[nodiscard]] std::vector<std::string> nonDistroRepos() const
    {
        return m_allRepoFilesNames
            | std::views::filter([&](const auto& filename) {
                  return !cloyster::functions::isIn(distroFilesNames, filename);
              })
            | std::ranges::to<std::vector>();
    }

    [[nodiscard]] std::vector<std::string> filterOracleLinux() const
    {
        return filterByFilename("oracle.repo");
    };

    [[nodiscard]] std::vector<std::string> filterRHEL() const
    {
        return filterByFilename("rhel.repo");
    };

    [[nodiscard]] std::vector<std::string> filterAlmaLinux() const
    {
        return filterByFilename("almalinux.repo");
    };

    [[nodiscard]] std::vector<std::string> filterRockyLinux() const
    {
        const auto shouldUseVault = ShouldUseVaultService::shouldUseVault(m_osinfo);
        return m_allRepoFilesNames
            | std::views::filter([&](const std::string& filenameInConf) {
                if (shouldUseVault) {
                  return  filenameInConf == "rocky-vault.repo";
                } else {
                  return filenameInConf == "rocky.repo";
                }
              })
            | std::ranges::to<std::vector>();
    };

    [[nodiscard]] std::vector<std::string> filterByFilename(
        const std::string_view& filename) const
    {
        return m_allRepoFilesNames
            | std::views::filter([&](const std::string& filenameInConf) {
                  return filenameInConf == filename;
              })
            | std::ranges::to<std::vector>();
    };
};

TEST_CASE("RepoFilter")
{
    // Log::init(5);
    const auto conffile = RepoConfigParser::parseTest("repos/repos.conf");
    const auto osinfo = OS(
        OS::Distro::Rocky,
        OS::Platform::el9,
        5
     );
    struct ShouldUseVaultService final {
        static bool shouldUseVault(const OS& /* osinfo */) { return false; }
    };
    const auto filter = RepoFilter<ShouldUseVaultService>(conffile, osinfo);
    const auto rockyRepoFiles = filter.distroRepos();
    const auto nonDistroRepoFiles = filter.nonDistroRepos();
    CHECK(rockyRepoFiles.size() == 1);
    CHECK(rockyRepoFiles[0] == "rocky.repo");
    for (const auto& nonDistroRepoFilename : nonDistroRepoFiles) {
        CHECK(!cloyster::functions::isIn(
            filter.distroFilesNames, nonDistroRepoFilename));
    }
}

// Return the repository names to enable based on the osinfo
template <typename ShouldUseVaultService = RockyLinux>
struct RepoNames {
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

    static std::vector<std::string> resolveReposNames(const OS& osinfo, const RepoConfFile& conffile)
    {
        auto distro = osinfo.getDistro();
        auto majorVersion = osinfo.getMajorVersion();
        auto output = std::vector<std::string>();
        const auto filter = RepoFilter<ShouldUseVaultService>(conffile, osinfo);
        const auto distroRepos = filter.distroRepos();
        const auto nonDistroRepos = filter.nonDistroRepos();
        const auto addToOutput = [&output]<typename ...T>(fmt::format_string<T...> fmt, T... args)
        {
            output.emplace_back(fmt::format(fmt::runtime(fmt), args...));
        };

        switch (distro) {
            case OS::Distro::AlmaLinux:
                addToOutput("AlmaLinuxBaseOS");
                addToOutput("\"AlmaLinux {releasever} - CRB\"",
                    fmt::arg("releasever", majorVersion));
                break;
            case OS::Distro::Rocky:
            {
                if (ShouldUseVaultService::shouldUseVault(osinfo)) {
                    addToOutput("RockyAppStream-vault");
                    addToOutput("RockyBaseOS-vault");
                    addToOutput("{}-vault", resolveCodeReadyBuilderName(osinfo));
                } else {
                    addToOutput("RockyAppStream");
                    addToOutput("RockyBaseOS");
                    addToOutput("{}", resolveCodeReadyBuilderName(osinfo));
                }
                break;
            }
            case OS::Distro::RHEL:
                addToOutput("rhel-{releasever}-baseos",
                    fmt::arg("releasever", majorVersion));
                addToOutput("rhel-{releasever}-codeready-builder",
                    fmt::arg("releasever", majorVersion));
                break;
            case OS::Distro::OL:
                addToOutput("OLBaseOS");
                addToOutput("ol{releasever}_codeready_builder",
                    fmt::arg("releasever", majorVersion));
                break;
            default:
                throw std::runtime_error("Unsupported distro");
        }
        addToOutput("epel");
        addToOutput("OpenHPC");
        addToOutput("OpenHPC-Updates");
        addToOutput("rpmfusion");
        addToOutput("elrepo");
        addToOutput("beegfs");
        return output;
    }

    static std::vector<std::string> resolveReposNames(const OS& osinfo, const RepoConfigVars& vars)
    {
        return resolveReposNames(osinfo, RepoConfigParser::parse(vars));
    }
};

TEST_CASE("RepoNames")
{
    struct ShouldUseVaultService final {
        static bool shouldUseVault(const OS& osinfo) { return false; }
    };
    const auto enabler = RepoNames<ShouldUseVaultService>{};
    const auto conffile = RepoConfigParser::parseTest("repos/repos.conf");

    // RHEL
    {
        const auto osinfo = OS(
            models::OS::Distro::RHEL,
            OS::Platform::el9,
            5
        );
        const auto enabledRepos = enabler.resolveReposNames(osinfo, conffile);
        // fmt::print("Repos: {}\n", fmt::join(enabledRepos, ","));
        CHECK(enabledRepos == std::vector<std::string> {
            "rhel-9-baseos",
            "rhel-9-codeready-builder",
            "epel",
            "OpenHPC",
            "OpenHPC-Updates",
            "rpmfusion",
            "elrepo",
            "beegfs",
        });
    }

    // AlmaLinux
    {
        const auto osinfo = OS(
            models::OS::Distro::AlmaLinux,
            OS::Platform::el9,
            5
        );
        const auto enabledRepos = enabler.resolveReposNames(osinfo, conffile);
        CHECK(enabledRepos == std::vector<std::string> {
            "AlmaLinuxBaseOS",
            "\"AlmaLinux 9 - CRB\"",
            "epel",
            "OpenHPC",
            "OpenHPC-Updates",
            "rpmfusion",
            "elrepo",
            "beegfs",
        });
    }

    // Rocky
    {
        const auto osinfo = OS(
            models::OS::Distro::Rocky,
            OS::Platform::el9,
            5
        );
        const auto enabledRepos = enabler.resolveReposNames(osinfo, conffile);
        // fmt::print("Repos: {}", fmt::join(enabledRepos, ","));
        CHECK(enabledRepos == std::vector<std::string> {
            "RockyAppStream",
            "RockyBaseOS",
            "crb",
            "epel",
            "OpenHPC",
            "OpenHPC-Updates",
            "rpmfusion",
            "elrepo",
            "beegfs",
        });
    }

    // OL
    {
        const auto osinfo = OS(
            models::OS::Distro::OL,
            OS::Platform::el9,
            5
        );
        const auto enabledRepos = enabler.resolveReposNames(osinfo, conffile);
        // fmt::print("Repos: {}", fmt::join(enabledRepos, ","));
        CHECK(enabledRepos == std::vector<std::string> {
            "OLBaseOS",
            "ol9_codeready_builder",
            "epel",
            "OpenHPC",
            "OpenHPC-Updates",
            "rpmfusion",
            "elrepo",
            "beegfs",
        });
    }
}

// Generate the repositories in the disk
template <typename MChecker = DefaultMirrorExistenceChecker,
    typename UChecker = DefaultMirrorExistenceChecker,
    typename ShouldUseVaultService = RockyLinux
>
struct RepoGenerator final {
    static std::size_t generate(const RepoConfFile& conffile,
        const OS& osinfo, const std::filesystem::path& path)
    {
        const auto existingRepoFiles
            = cloyster::functions::getFilesByExtension(path, ".repo");
        const auto filter = RepoFilter<ShouldUseVaultService>(conffile, osinfo);
        const auto distroRepos = filter.distroRepos();
        const auto nonDistroRepos = filter.nonDistroRepos();
        std::vector<std::string> reposToGenerate;
        for (const auto& repo : distroRepos) {
            if (!cloyster::functions::isIn(existingRepoFiles, repo)) {
                reposToGenerate.push_back(repo);
            }
        }
        for (const auto& repo : nonDistroRepos) {
            if (!cloyster::functions::isIn(existingRepoFiles, repo)) {
                reposToGenerate.push_back(repo);
            }
        }
        const std::vector<RPMRepositoryFile> repofiles
            = RepoConfAdapter<MChecker, UChecker>::fromConfFile(
                conffile, reposToGenerate, path);
        for (const auto& repofile : repofiles) {
            repofile.save();
        }

        return repofiles.size();
    }

    static std::size_t generate(
        const OS& osinfo,
        const RepoConfigVars& vars)
    {
        const auto conffile = RepoConfigParser::parse(vars);
        return generate(conffile, osinfo, RPMRepoManager::basedir);
    }
};

TEST_CASE("RepoGenerator")
{
    auto opts = Options{
        .disableMirrors = false,
        .mirrorBaseUrl = "https://mirror.example.com",
    };
    cloyster::Singleton<Options>::init(std::make_unique<Options>(opts));
    const auto upstreamPath = "test/output/repos/upstream";
    const auto mirrorPath = "test/output/repos/mirror";
    const auto airgapPath = "test/output/repos/airgap";
    const auto conffile = RepoConfigParser::parseTest("repos/repos.conf");

    // Clean up before start
    for (const auto& path : {upstreamPath, mirrorPath, airgapPath}) {
        cloyster::functions::removeFilesWithExtension(path, ".repo");
    }

    const auto osinfo = OS(
        models::OS::Distro::Rocky,
        OS::Platform::el9,
        5
    );

    const auto generator = RepoGenerator<
        FalseMirrorExistenceChecker, // mirror
        TrueMirrorExistenceChecker  // upstream
    >();
    const auto generatedCount1
        = generator.generate(conffile, osinfo, upstreamPath);
    CHECK(generatedCount1 == 14);

    const auto generatedCount2
        = generator.generate(conffile, osinfo, upstreamPath);
    // It does not re-generate the files in the second run
    CHECK(generatedCount2 == 0);

    // Generate the other files so we can look at them
    const auto generatorMirror = RepoGenerator<
        TrueMirrorExistenceChecker, // mirror
        TrueMirrorExistenceChecker  // upstream
    >();
    generatorMirror.generate(conffile, osinfo, mirrorPath);
    cloyster::Singleton<Options>::get()->mirrorBaseUrl = "file:///var/run/repos";
    generatorMirror.generate(conffile, osinfo, airgapPath);
};

TEST_SUITE("repos urls")
{
    // RH CDN requires a certificate that only exists in RHEL machines
    // because of this the repostiories gives 403 and SSL errors. I'm skipping
    // them for now.
    constexpr auto blacklistedFiles = {
        "rhel.repo",
    };

    constexpr auto blacklistedMirrorFiles = {
        "almalinux.repo",
        "nvidia.repo",
        "influxdata.repo",
        "mlx-doca.repo",
    };

    TEST_CASE("[slow] repo.conf urls")
    {
        using namespace cloyster::services;
        Options opts{};
        cloyster::services::initializeSingletonsOptions(
            std::make_unique<Options>(opts));
        // Log::init(5);
        RepoConfFile output =  RepoConfigParser::parseTest("repos/repos.conf");
        for (const auto& [repofile, configs] : output.files()) {
            for (const auto& config : configs) {
                if (cloyster::functions::isIn(
                        blacklistedFiles, config.repoId.filename)) {

                    continue;
                }

                LOG_INFO("Checking {}", config.repoId.id);

                REQUIRE(cloyster::functions::getHttpStatus(
                            config.upstream.repo + "repodata/repomd.xml")
                    == "200");

                if (config.upstream.gpgkey) {
                    REQUIRE(cloyster::functions::getHttpStatus(
                                config.upstream.gpgkey.value())
                        == "200");
                }

                if (cloyster::functions::isIn(
                        blacklistedMirrorFiles, config.repoId.filename)) {
                    continue;
                }

                REQUIRE(cloyster::functions::getHttpStatus(
                            "https://mirror.versatushpc.com.br/"
                            + config.mirror.repo + "repodata/repomd.xml")
                    == "200");

                if (config.mirror.gpgkey) {
                    REQUIRE(cloyster::functions::getHttpStatus(
                                "https://mirror.versatushpc.com.br/"
                                + config.mirror.gpgkey.value())
                        == "200");
                }
            }
        }
    }
}
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

struct RPMRepositoryGenerator {
    static void generate(const RepoConfigVars& vars,
        const std::filesystem::path& backupPath = "/opt/cloysterhpc/backup/etc/yum.repos.d/",
        const std::filesystem::path& sourcePath = "/etc/yum.repos.d"
        )
    {
        namespace fs = std::filesystem;
        cloyster::functions::backupFilesByExtension(
            wrappers::DestinationPath(backupPath),
            wrappers::SourcePath(sourcePath),
            wrappers::Extension(".repo")
        );
        LOG_DEBUG("Generating the repository files");
        const auto cluster = cloyster::Singleton<models::Cluster>::get();
        const auto osinfo = cluster->getHeadnode().getOS();
        RepoGenerator<>::generate(osinfo, vars);
    }
};

void RepoManager::initializeDefaultRepositories()
{
    auto opts = cloyster::Singleton<Options>::get();
    if (opts->dryRun) {
        LOG_WARN("Dry Run: Skipping RepoManager initialization");
        return;
    }
    LOG_INFO("RepoManager initialization");
    auto osinfo
        = cloyster::Singleton<models::Cluster>::get()->getHeadnode().getOS();

    const auto vars = RepoConfigVars {
        .arch = cloyster::utils::enums::toString(osinfo.getArch()),
        .beegfsVersion = opts->beegfsVersion,
        .ohpcVersion = osinfo.getMajorVersion() == 8 ? "2" : "3",
        .osversion = osinfo.getVersion(),
        .releasever = fmt::format("{}", osinfo.getMajorVersion()),
        .xcatVersion = opts->xcatVersion,
        .zabbixVersion = opts->zabbixVersion,
    };
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            {
                // Generate the repository files
                RPMRepositoryGenerator::generate(vars);
                // Get the names of repositories to enable
                const auto repos = RepoNames<>::resolveReposNames(osinfo, vars);
                // Load the base directory, /etc/yum.repos.d/*.repo files
                m_impl->rpm.loadBaseDir();
                // Enable the repositories
                m_impl->rpm.enable(repos, true);
            }
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

    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            m_impl->rpm.enable(repoid, true);
            break;
        default:
            throw std::logic_error("Not implemented");
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
    switch (osinfo.getPackageType()) {
        case OS::PackageType::RPM:
            m_impl->rpm.enable(repos, true);
            break;
        default:

            throw std::logic_error("Not implemented");
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

TEST_SUITE_END();

}; // namespace cloyster::services::repos
