/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/inifile.h>
#include <cloysterhpc/repos.h>
#include <cloysterhpc/runner.h>
#include <cloysterhpc/services/log.h>
#include <filesystem>
#include <fstream>
#include <magic_enum.hpp>
#include <ranges>

#include <boost/algorithm/string.hpp>

constexpr std::string_view CLOYSTER_REPO_EL8 {
#include "cloysterhpc/repos/el8/cloyster.repo"
};

constexpr std::string_view CLOYSTER_REPO_EL9 = {
#include "cloysterhpc/repos/el9/cloyster.repo"
};

static repository loadSection(const std::filesystem::path& source,
    inifile& file, const std::string& section)
{
    auto name = file.getValue(section, "name");
    auto baseurl = file.getValue(section, "baseurl");
    auto enabled = file.getValue(section, "enabled");
    auto gpgcheck = file.getValue(section, "gpgcheck");
    auto gpgkey = file.getValue(section, "gpgkey");

    boost::trim(name);
    boost::trim(baseurl);
    boost::trim(enabled);
    boost::trim(gpgcheck);
    boost::trim(gpgkey);

    LOG_DEBUG("found repo <{}> (id {}) at {}", name, section, source.string());

    repository r = { .id = section,
        .enabled = (enabled == "1"),
        .name = name,
        .baseurl = baseurl,
        .metalink = "",
        .gpgcheck = (gpgcheck == "1"),
        .gpgkey = gpgkey,
        .source = source };

    return r;
}

static void writeSection(inifile& file, const repository& repo)
{
    std::string section = repo.id;

    file.setValue(section, "name", repo.name);
    file.setValue(section, "baseurl", repo.baseurl);
    file.setValue(section, "enabled", repo.enabled ? "1" : "0");
    file.setValue(section, "gpgcheck", repo.gpgcheck ? "1" : "0");
    file.setValue(section, "gpgkey", repo.gpgkey);

    LOG_INFO("writing repo <{}> (id {}) at {}", repo.name, section,
        repo.source.string());
}

static void loadFromINI(const std::filesystem::path& source, inifile& file,
    std::vector<repository>& out)
{
    auto sections = file.listAllSections();

    for (const auto& reponame : sections) {
        auto r = loadSection(source, file, reponame);
        out.push_back(r);
    }
}

#define NOSONAR(code) code

void RepoManager::loadSingleFile(std::filesystem::path source)
{
    inifile file;
    file.loadFile(source);
    loadFromINI(source, file, m_repos);
}

void RepoManager::loadFiles(const std::filesystem::path& basedir)
{
    for (auto const& dir_entry :
        std::filesystem::directory_iterator { basedir }) {
        const auto path = dir_entry.path();
        if (path.extension() == ".repo")
            loadSingleFile(path);
    }

    auto cloyster_repos = buildCloysterTree(basedir);
    mergeWithCurrentList(std::move(cloyster_repos));

    auto destparent
        = NOSONAR(std::filesystem::temp_directory_path()) / "cloyster0";
    auto destination = destparent / "yum.repos.d";
    std::filesystem::create_directory(destparent);
    std::filesystem::create_directory(destination);

    configureEL();
    configureXCAT(destination);

    for (auto const& dir_entry :
        std::filesystem::directory_iterator { destination }) {
        const auto path = dir_entry.path();
        if (path.extension() == ".repo")
            loadSingleFile(path);
    }
}

void RepoManager::loadCustom(inifile& file, const std::filesystem::path& path)
{
    std::vector<repository> data;
    loadFromINI(path, file, data);
    mergeWithCurrentList(std::move(data));
}

void RepoManager::setEnableState(const std::string& id, bool value)
{
    for (auto& repo : m_repos) {
        if (repo.id == id) {
            repo.enabled = value;
            return;
        }
    }
}

void RepoManager::enable(const std::string& id) { setEnableState(id, true); }

void RepoManager::enableMultiple(std::vector<std::string> ids)
{
    std::ranges::for_each(ids, [&](const auto& id) { this->enable(id); });
}

void RepoManager::disable(const std::string& id) { setEnableState(id, false); }

static std::string buildPackageName(std::string stem)
{
    return fmt::format("{}{}", cloyster::productName, stem);
}

static std::vector<std::string> getDependenciesEL(
    const OS& os, OS::Platform version)
{
    std::vector<std::string> dependencies;

    const char* powertools
        = version == OS::Platform::el8 ? "powertools" : "crb";
    int numversion = version == OS::Platform::el8 ? 8 : 9;

    switch (os.getDistro()) {
        case OS::Distro::AlmaLinux:
            dependencies
                = { buildPackageName("-AlmaLinux-BaseOS"), powertools };
            break;
        case OS::Distro::RHEL:
            dependencies = { fmt::format(
                "codeready-builder-for-rhel-{}-x86_64-rpms", numversion) };
            break;
        case OS::Distro::OL:
            dependencies = { buildPackageName("-OL-BaseOS"),
                fmt::format("ol{}_codeready_builder", numversion) };
            break;
        case OS::Distro::Rocky:
            dependencies = { buildPackageName("-Rocky-BaseOS"), powertools };
            break;
        default:
            throw std::runtime_error("Unsupported platform");
    }

    return dependencies;
}

void RepoManager::configureEL()
{
    std::vector<std::string> deps;
    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
        case OS::Platform::el9:
            deps = getDependenciesEL(m_os, m_os.getPlatform());
            break;
        default:
            throw std::runtime_error("Unsupported platform");
    }

    std::for_each(
        deps.begin(), deps.end(), [this](const auto& id) { this->enable(id); });
}

void RepoManager::commitStatus()
{
    m_runner.executeCommand("dnf -y install initscripts");
    createFileFor("/etc/yum.repos.d/cloyster.repo");
    auto tmpdir = NOSONAR(std::filesystem::temp_directory_path())
        / "cloyster0/yum.repos.d";

    for (auto const& dir_entry :
        std::filesystem::directory_iterator { tmpdir }) {
        std::filesystem::copy(dir_entry, "/etc/yum.repos.d");
    }

    std::vector<std::string> to_enable;
    std::vector<std::string> to_disable;

    for (const auto& repo : m_repos) {
        if (repo.enabled) {
            to_enable.push_back(repo.id);
        } else {
            to_disable.push_back(repo.id);
        }
    }

    if (!to_enable.empty()) {
        m_runner.executeCommand(
            fmt::format("sudo dnf config-manager --set-enabled {}",
                fmt::join(to_enable, ",")));
    }

    if (!to_disable.empty()) {
        m_runner.executeCommand(
            fmt::format("sudo dnf config-manager --set-disabled {}",
                fmt::join(to_disable, ",")));
    }
}

#define FORMAT_TEMPLATE(src) fmt::format(src, cloyster::productName)

std::vector<repository> RepoManager::buildCloysterTree(
    const std::filesystem::path& basedir)
{

    std::vector<repository> cloyster_repos;
    inifile file;

    if (cloyster::customRepofilePath.empty()) {
        switch (m_os.getPlatform()) {
            case OS::Platform::el8:
                file.loadData(FORMAT_TEMPLATE(CLOYSTER_REPO_EL8));
                break;
            case OS::Platform::el9:
                file.loadData(FORMAT_TEMPLATE(CLOYSTER_REPO_EL9));
                break;
            default:
                throw std::runtime_error("Unsupported platform");
        }

    } else {
        LOG_INFO("Using custom repofile ({}).", cloyster::customRepofilePath);
        file.loadFile(cloyster::customRepofilePath);
    }

    auto outpath = basedir / "cloyster.repo";
    loadFromINI(outpath, file, cloyster_repos);

    return cloyster_repos;
}

void RepoManager::createFileFor(std::filesystem::path path)
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create file {}", path.string());
        return;
    }

    if (std::filesystem::exists(path)) {
        LOG_INFO("repository file {} already exists...", path.string());
        return;
    }

    inifile file;

    auto filtered = m_repos
        | std::views::filter([&path](auto& r) { return path == r.source; });

    for (const auto& repo : filtered) {
        writeSection(file, repo);
    }

    file.saveFile(path);
}

void RepoManager::mergeWithCurrentList(std::vector<repository>&& repo)
{
    for (auto&& r : repo) {
        if (std::find_if(m_repos.begin(), m_repos.end(),
                [&](auto& v) { return v.id == r.id; })
            == m_repos.end()) {
            m_repos.push_back(r);
        }
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

void RepoManager::configureXCAT(const std::filesystem::path& repofile_dest)
{
    LOG_INFO("Setting up XCAT repositories");

    // TODO: we need to download these files in a sort of temporary directory
    m_runner.downloadFile("https://xcat.org/files/xcat/repos/yum/latest/"
                          "xcat-core/xcat-core.repo",
        repofile_dest.string());

    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            m_runner.downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh8/x86_64/xcat-dep.repo",
                repofile_dest.string());
            break;
        case OS::Platform::el9:
            m_runner.downloadFile(
                "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                "rh9/x86_64/xcat-dep.repo",
                repofile_dest.string());
            break;
        default:
            throw std::runtime_error("Unsupported platform for xCAT");
    }
}

std::vector<std::string> RepoManager::getxCATOSImageRepos() const
{
    const auto osArch = magic_enum::enum_name(m_os.getArch());
    const auto osMajorVersion = m_os.getMajorVersion();
    const auto osVersion = m_os.getVersion();

    std::vector<std::string> repos;

    std::vector<std::string> latestEL = { "8.9", "9.3" };

    std::string crb = "CRB";
    std::string rockyBranch
        = "linux"; // To check if Rocky mirror directory points to 'linux'
                   // (latest version) or 'vault'

    std::string OpenHPCVersion = "3";

    if (osMajorVersion < 9) {
        crb = "PowerTools";
        OpenHPCVersion = "2";
    }

    if (std::find(latestEL.begin(), latestEL.end(), osVersion)
        == latestEL.end()) {
        rockyBranch = "vault";
    }

    switch (m_os.getDistro()) {
        case OS::Distro::RHEL:
            repos.emplace_back(
                "https://cdn.redhat.com/content/dist/rhel8/8/x86_64/baseos/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/appstream/os");
            repos.emplace_back("https://cdn.redhat.com/content/dist/rhel8/8/"
                               "x86_64/codeready-builder/os");
            break;
        case OS::Distro::OL:
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/oracle/{}/"
                            "baseos/latest/{}",
                    osMajorVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/oracle/{}/appstream/{}",
                osMajorVersion, osArch));
            repos.emplace_back(fmt::format("https://mirror.versatushpc.com.br/"
                                           "oracle/{}/codeready/builder/{}",
                osMajorVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/oracle/{}/UEKR7/{}",
                osMajorVersion, osArch));
            break;
        case OS::Distro::Rocky:
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/BaseOS/{}/os",
                rockyBranch, osVersion, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/{}/{}/os",
                rockyBranch, osVersion, crb, osArch));
            repos.emplace_back(fmt::format(
                "https://mirror.versatushpc.com.br/rocky/{}/{}/AppStream/{}/os",
                rockyBranch, osVersion, osArch));
            break;
        case OS::Distro::AlmaLinux:
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/almalinux/"
                            "almalinux/{}/BaseOS/{}/os",
                    osVersion, osArch));
            repos.emplace_back(fmt::format("https://mirror.versatushpc.com.br/"
                                           "almalinux/almalinux/{}/{}/{}/os",
                osVersion, crb, osArch));
            repos.emplace_back(
                fmt::format("https://mirror.versatushpc.com.br/almalinux/"
                            "almalinux/{}/AppStream/{}/os",
                    osVersion, osArch));
            break;
    }

    repos.emplace_back(
        fmt::format("https://mirror.versatushpc.com.br/epel/{}/Everything/{}",
            osMajorVersion, osArch));

    // Modular repositories are only available on EL8
    if (osMajorVersion == 8) {
        repos.emplace_back(
            fmt::format("https://mirror.versatushpc.com.br/epel/{}/Modular/{}",
                osMajorVersion, osArch));
    }

    repos.emplace_back(
        fmt::format("https://mirror.versatushpc.com.br/openhpc/{}/EL_{}",
            OpenHPCVersion, osMajorVersion));
    repos.emplace_back(fmt::format(
        "https://mirror.versatushpc.com.br/openhpc/{}/updates/EL_{}",
        OpenHPCVersion, osMajorVersion));

    return repos;
}

const std::vector<repository>& RepoManager::listRepos() const
{
    return m_repos;
}

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <cloysterhpc/tempdir.h>
#include <cloysterhpc/tests.h>

TEST_SUITE("Test repository file read and write")
{
    TEST_CASE(
        "Check if the default cloyster repository file is correctly parsed")
    {
        MockRunner mr;
        OS osinfo { OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el9,
            OS::Distro::Rocky, "6.69.6969", 9, 9 };

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
    }

    TEST_CASE("Check if the repository enable operations are run")
    {
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
    }
}
