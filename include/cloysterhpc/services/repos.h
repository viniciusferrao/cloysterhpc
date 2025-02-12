/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */


/* This class is set to be reimplemented */
#ifndef CLOYSTERHPC_REPOS_H_DEPRECATED_
#define CLOYSTERHPC_REPOS_H_DEPRECATED_

#include <filesystem>
#include <string>

#include <cloysterhpc/inifile.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/runner.h>
#include <cloysterhpc/services/repofile.h>

namespace cloyster {
extern std::string customRepofilePath;
};

namespace cloyster::services::repos {

class RepoManager {
    using OS = cloyster::models::OS;
public:
    RepoManager(BaseRunner& runner, const OS& osinfo);
    void loadFiles(const std::filesystem::path& basedir = "/etc/yum.repos.d");
    void commitStatus();
    void enable(const std::string& repo);
    void enableMultiple(std::vector<std::string> repos);
    void disable(const std::string& repo);
    [[nodiscard]] const std::vector<BaseRepository>& listRepos() const;
    [[nodiscard]] std::vector<std::string> getxCATOSImageRepos() const;

private:
    void loadCustom(inifile& file, const std::filesystem::path& path);
    // BUG: Enable and EnableMultiple are the same method. Overload it.
    std::vector<BaseRepository> m_repos;

    BaseRunner& m_runner;
    const OS& m_os;

    void createFileFor(std::filesystem::path path);

    // Repository type is required to be known here so 
    // it can be constructed by buildCloysterTree 
    template <IsRepository Repository>

    std::vector<BaseRepository> buildCloysterTree(
        const std::filesystem::path& basedir);

    void loadSingleFile(const std::filesystem::path& source);

    void saveRepositories();

    void configureXCAT(const std::filesystem::path& repofile_dest);
    void configureEL();
    void setEnableState(const std::string& id, bool value);

    template <IsRepository Repository>
    void mergeWithCurrentList(std::vector<Repository>&& repo);
};

};
#endif // CLOYSTERHPC_REPOS_H_DEPRECATED_
