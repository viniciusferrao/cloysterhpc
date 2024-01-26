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

Repos::Repos(OS::Distro distro, OS::Platform platform)
    : m_distro(distro)
    , m_platform(platform)
{
    switch (m_platform) {
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

void Repos::configureOL() const {
    createGPGKeyFile(m_family.Oracle.repo_gpg_filename, m_family.Oracle.repo_gpg);
    enable(m_family.Oracle.joinDependencies());
}

void Repos::configureRocky() const
{
    createGPGKeyFile(m_family.Rocky.repo_gpg_filename, m_family.Rocky.repo_gpg);
    enable(m_family.Rocky.joinDependencies());
}

void Repos::configureAlma() const
{
    createGPGKeyFile(m_family.AlmaLinux.repo_gpg_filename, m_family.AlmaLinux.repo_gpg);
    enable(m_family.AlmaLinux.joinDependencies());
}

void Repos::configureXCAT() const
{
    LOG_INFO("Setting up XCAT repositories")

    runCommand("wget -NP /etc/yum.repos.d "
               "https://xcat.org/files/xcat/repos/yum/latest/"
               "xcat-core/xcat-core.repo");

    switch (m_platform) {
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

void Repos::configureRepositories() const
{
    LOG_INFO("Setting up repositories")

    createCloysterRepo();

    switch (m_distro) {
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
