/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOS_H_
#define CLOYSTERHPC_REPOS_H_

#include <cloysterhpc/inifile.h>
#include <cloysterhpc/os.h>
#include <cloysterhpc/runner.h>

#include <filesystem>
#include <ranges>
#include <string>

namespace cloyster {
extern std::string customRepofilePath;
};

struct repository {
    std::string id;
    bool enabled = true;
    std::string name;
    std::string baseurl;
    std::string metalink;
    bool gpgcheck = true;
    std::string gpgkey;
    std::filesystem::path source;
};

class RepoManager {
public:
    enum class AdditionalType {
        beegfs,
        ELRepo,
        EPEL,
        Grafana,
        influxData,
        oneAPI,
        OpenHPC,
        Zabbix,
        RPMFusionUpdates
    };

    RepoManager(BaseRunner& runner, const OS& osinfo)
        : m_runner(runner)
        , m_os(osinfo)
    {
    }

    void loadFiles(const std::filesystem::path& basedir = "/etc/yum.repos.d");
    void loadCustom(inifile& file, const std::filesystem::path& path);

    void enable(const std::string& id);
    void enableMultiple(std::vector<std::string> ids);
    void disable(const std::string& id);

    void commitStatus();

    const std::vector<repository>& listRepos() const;

    std::vector<std::string> getxCATOSImageRepos() const;

private:
    std::vector<repository> m_repos;
    BaseRunner& m_runner;
    OS m_os;

    void createFileFor(std::filesystem::path path);

    std::vector<repository> buildCloysterTree(
        const std::filesystem::path& basedir);

    void loadSingleFile(std::filesystem::path source);
    void saveRepositories();

    void configureXCAT(const std::filesystem::path& repofile_dest);
    void configureEL();
    void setEnableState(const std::string& id, bool value);

    void mergeWithCurrentList(std::vector<repository>&& repo);
};

#endif // CLOYSTERHPC_REPOS_H_
