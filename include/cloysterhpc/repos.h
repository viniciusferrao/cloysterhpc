/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOS_H_
#define CLOYSTERHPC_REPOS_H_

#include <cloysterhpc/os.h>
#include <string>
#include <cloysterhpc/repos/repoimport.h>

struct repofile {
    std::string id;
    bool enabled = true;
    std::string name;
    std::string baseurl;
    std::string metalink;
    bool gpgcheck = true;
    std::string gpgkey;
    std::string gpgkeyContent;

    repofile(const std::string& id, const std::string& name,
        const std::string& baseurl, const std::string& metalink,
        const std::string& gpgkey, const std::string& gpgkeyContent)
        : id(id)
        , name(name)
        , baseurl(baseurl)
        , metalink(metalink)
        , gpgkey(gpgkey)
        , gpgkeyContent(gpgkeyContent)
    {
    }
};

class Repos {
private:
    void configureRHEL() const;
    void configureRocky() const;
    void configureOL() const;
    void configureAlma() const;
    void configureXCAT() const;
    void configureAdditionalRepos(const std::vector<AdditionalType>& additional) const;
    void createGPGKeyFile(const std::string& filename, const std::string& key) const;
    void createGPGKeyFile(const std::filesystem::path& path, const std::string& key)const;
    void createGPGKeyFile(const repofile& repo) const;
    void createCloysterRepo() const;
    OS m_os;
    Family m_family;


public:
    explicit Repos(const OS& osinfo);
    void createConfigurationFile(const repofile& repo) const;
    static void enable(const std::string& id);
    static void disable(const std::string& id);
    void configureRepositories() const;
    [[nodiscard]] std::vector<std::string> getxCATOSImageRepos() const;
};

#endif // CLOYSTERHPC_REPOS_H_