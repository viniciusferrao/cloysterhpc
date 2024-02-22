/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/repos.h>
#include <cloysterhpc/services/log.h>
#include <fstream>
#include <magic_enum.hpp>

using boost::property_tree::ptree;
using cloyster::runCommand;

Repos::Repos(const OS& m_os)
    : m_os(m_os)
{
    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            m_family = EL8;
            break;
        case OS::Platform::el9:
            m_family = EL9;
            break;
        default:
            throw std::runtime_error("Unsupported platform");
    }
}

void Repos::createConfigurationFile(const repofile& repo) const
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create repofile {}", repo.name)
        return;
    }

    ptree repof {};

    repof.put(fmt::format("{}.name", repo.id), repo.name);
    repof.put(fmt::format("{}.gpgcheck", repo.id), repo.gpgcheck ? 1 : 0);
    repof.put(fmt::format("{}.gpgkey", repo.id), repo.gpgkey);
    repof.put(fmt::format("{}.enabled", repo.id), repo.enabled ? 1 : 0);
    repof.put(fmt::format("{}.baseurl", repo.id), repo.baseurl);

    boost::property_tree::ini_parser::write_ini(
        fmt::format("/etc/yum.repos.d/{}.repo", repo.id), repof);

    createGPGKeyFile(repo);
    LOG_INFO("Created repofile {}", repo.name)
}

void Repos::createGPGKeyFile(const repofile& repo) const
{
    std::filesystem::path path = repo.gpgkey.substr(7);
    createGPGKeyFile(path, repo.gpgkeyContent);
}

void Repos::createGPGKeyFile(
    const std::string& filename, const std::string& key) const
{
    std::filesystem::path path = fmt::format("/etc/pki/rpm-gpg/{}", filename);
    createGPGKeyFile(path, key);
}

void Repos::createGPGKeyFile(
    const std::filesystem::path& path, const std::string& key) const
{
    if (cloyster::dryRun) {
        LOG_INFO("Would create GPG Key file {}", path.filename().string())
        return;
    }

    std::ofstream gpgkey(path);
    gpgkey << key;
    gpgkey.close();
}

void Repos::enable(const std::string& id)
{
    runCommand(fmt::format("sudo dnf config-manager --set-enabled {}", id));
}

void Repos::disable(const std::string& id)
{
    runCommand(fmt::format("sudo dnf config-manager --set-disabled {}", id));
}

void Repos::configureRHEL() const { enable(m_family.RHEL.joinDependencies()); }

void Repos::configureOL() const
{
    createGPGKeyFile(
        m_family.Oracle.repo_gpg_filename, m_family.Oracle.repo_gpg);
    enable(m_family.Oracle.joinDependencies());
}

void Repos::configureRocky() const
{
    disable("baseos");
    createGPGKeyFile(m_family.Rocky.repo_gpg_filename, m_family.Rocky.repo_gpg);
    enable(m_family.Rocky.joinDependencies());
}

void Repos::configureAlma() const
{
    createGPGKeyFile(
        m_family.AlmaLinux.repo_gpg_filename, m_family.AlmaLinux.repo_gpg);
    enable(m_family.AlmaLinux.joinDependencies());
}

void Repos::configureXCAT() const
{
    LOG_INFO("Setting up XCAT repositories")

    runCommand("wget -NP /etc/yum.repos.d "
               "https://xcat.org/files/xcat/repos/yum/latest/"
               "xcat-core/xcat-core.repo");

    switch (m_os.getPlatform()) {
        case OS::Platform::el8:
            runCommand("wget -NP /etc/yum.repos.d "
                       "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                       "rh8/x86_64/xcat-dep.repo");
            break;
        case OS::Platform::el9:
            runCommand("dnf -y install initscripts");
            runCommand("wget -NP /etc/yum.repos.d "
                       "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
                       "rh9/x86_64/xcat-dep.repo");
            break;
        default:
            throw std::runtime_error("Unsupported platform for xCAT");
    }
}

std::vector<std::string> Repos::getxCATOSImageRepos() const
{
    const auto osArch = magic_enum::enum_name(m_os.getArch());
    const auto osMajorVersion = m_os.getMajorVersion();
    const auto osVersion = m_os.getVersion();

    std::vector<std::string> repos;

    std::string latestEL = "9.3";

    std::string crb = "CRB";
    std::string rockyBranch
        = "linux"; // To check if Rocky mirror directory points to 'linux'
                   // (latest version) or 'vault'

    std::string OpenHPCVersion = "3";

    if (osMajorVersion < 9) {
        crb = "PowerTools";
        OpenHPCVersion = "2";
    }

    if (osVersion != latestEL) {
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

    /* TODO: if OpenHPC statement */
    repos.emplace_back(
        fmt::format("https://mirror.versatushpc.com.br/openhpc/{}/EL_{}",
            OpenHPCVersion, osMajorVersion));
    repos.emplace_back(fmt::format(
        "https://mirror.versatushpc.com.br/openhpc/{}/updates/EL_{}",
        OpenHPCVersion, osMajorVersion));

    return repos;
}

void Repos::configureRepositories() const
{
    LOG_INFO("Setting up repositories")

    createCloysterRepo();

    switch (m_os.getDistro()) {
        using enum OS::Distro;
        case RHEL:
            configureRHEL();
            break;
        case OL:
            configureOL();
            break;
        case Rocky:
            configureRocky();
            break;
        case AlmaLinux:
            configureAlma();
            break;
    }

    //@TODO Let user choose the optional repos.
    configureAdditionalRepos(
        { AdditionalType::beegfs, AdditionalType::ELRepo, AdditionalType::EPEL,
            AdditionalType::Grafana, AdditionalType::influxData,
            AdditionalType::oneAPI, AdditionalType::OpenHPC,
            AdditionalType::Zabbix, AdditionalType::RPMFusionUpdates });

    configureXCAT();
}

void Repos::createCloysterRepo() const
{
    LOG_INFO("Creating Cloyster repo")
    std::filesystem::path path = "/etc/yum.repos.d/cloyster.repo";

    std::ofstream repofile(path);
    repofile << m_family.repo;
    repofile.close();
}

void Repos::configureAdditionalRepos(
    const std::vector<AdditionalType>& additional) const
{
    LOG_INFO("Setting up additional repositories")

    for (AdditionalType type : additional) {
        for (const AdditionalRepo& repo : m_family.additionalRepos) {
            if (repo.type == type) {
                createGPGKeyFile(repo.gpg_filename, repo.gpg_key);
            }
        }
    }
}
