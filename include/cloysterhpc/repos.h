/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */


/* This class is set to be reimplemented */
#ifndef CLOYSTERHPC_REPOS_H_DEPRECATED_
#define CLOYSTERHPC_REPOS_H_DEPRECATED_

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

template <typename Repository, typename Runner>
class /* [[deprecated("refactoring")]] */ RepoManager {
public:
    RepoManager(Runner& runner, const OS& osinfo);
    void loadFiles(const std::filesystem::path& basedir = "/etc/yum.repos.d");
    void commitStatus();
    void enable(const std::string& repo);
    void enableMultiple(std::vector<std::string> repos);
    void disable(const std::string& repo);
    const std::vector<Repository>& listRepos() const;
    [[nodiscard]] std::vector<std::string> getxCATOSImageRepos() const;

private:
    void loadCustom(inifile& file, const std::filesystem::path& path);
    // BUG: Enable and EnableMultiple are the same method. Overload it.
    std::vector<Repository> m_repos;

    BaseRunner& m_runner;
    const OS& m_os;

    void createFileFor(std::filesystem::path path);

    const std::vector<Repository> buildCloysterTree(
        const std::filesystem::path& basedir);

    void loadSingleFile(std::filesystem::path source);
    void saveRepositories();

    void configureXCAT(const std::filesystem::path& repofile_dest);
    void configureEL();
    void setEnableState(const std::string& id, bool value);

    void mergeWithCurrentList(std::vector<Repository>&& repo);
};

#endif // CLOYSTERHPC_REPOS_H_DEPRECATED_
