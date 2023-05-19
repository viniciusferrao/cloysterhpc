/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "repos.h"
#include "functions.h"
#include "services/log.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>

using boost::property_tree::ptree;
using cloyster::runCommand;

Repos::Repos(OS::Distro distro)
    : m_distro(distro)
{
}

void Repos::createConfigurationFile(const repofile& repo)
{
    ptree repof;

    repof.put("baseos.name", repo.name);
    repof.put("baseos.gpgcheck", repo.gpgcheck);
    repof.put("baseos.gpgkey", repo.gpgkey);
    repof.put("baseos.enabled", repo.enabled);
    repof.put("baseos.baseurl", repo.baseurl);

    boost::property_tree::ini_parser::write_ini(
        fmt::format("/etc/yum.repos.d/{}.repo", repo.id), repof);

    std::ofstream gpgkey(repo.gpgkey.substr(7));
    gpgkey << repo.gpgkeyContent;
    gpgkey.close();
}

void Repos::enable(const std::string& id) { }

void Repos::disable(const std::string& id) { }

void Repos::configureRHEL()
{
    runCommand("dnf config-manager --set-enabled "
               "codeready-builder-for-rhel-8-x86_64-rpms");
}

void Repos::configureOL()
{
    runCommand("dnf config-manager --set-enabled "
               "ol8_codeready_builder");

    createConfigurationFile(ol::ol8_base_latest);
}

void Repos::configureRocky()
{
    runCommand("dnf config-manager --set-enabled "
               "powertools");

    createConfigurationFile(rocky::rocky8_baseos);
}

void Repos::configureXCAT()
{
    LOG_INFO("Setting up XCAT repositories");

    runCommand("wget -NP /etc/yum.repos.d "
               "https://xcat.org/files/xcat/repos/yum/latest/"
               "xcat-core/xcat-core.repo");
    runCommand("wget -NP /etc/yum.repos.d "
               "https://xcat.org/files/xcat/repos/yum/devel/xcat-dep/"
               "rh8/x86_64/xcat-dep.repo");
}

void Repos::configureRepositories()
{
    LOG_INFO("Setting up additional repositories");

    runCommand("dnf -y install "
               "https://dl.fedoraproject.org/pub/epel/"
               "epel-release-latest-8.noarch.rpm");
    runCommand("dnf -y install "
               "http://repos.openhpc.community/OpenHPC/2/CentOS_8/x86_64/"
               "ohpc-release-2-1.el8.x86_64.rpm");

    switch (m_distro) {
        case OS::Distro::RHEL:
            configureRHEL();
            break;
        case OS::Distro::OL:
            configureOL();
            break;
        case OS::Distro::Rocky:
            configureRocky();
            break;
    }

    configureXCAT();
}
