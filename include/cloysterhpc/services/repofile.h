/*
 * Copyright 2025 Arthur Mendes <arthurmco@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPOFILE_H_
#define CLOYSTERHPC_REPOFILE_H_

#include <vector>

#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

#include <cloysterhpc/file.h>
#include <cloysterhpc/services/repo.h>

struct ELCloneRepo {
    std::string group;
    std::string name;
    std::optional<std::string> base_url;
    std::optional<std::string> metalink;
    bool enabled;
    bool gpgcheck;
    std::string gpgkey;

    // (P/ todos os repositórios)
    // std::string release;
};

/**
 * Repository file class
 *
 * This class should parse the repository data
 */
class ELRepoFile : GenericFile {
private:
    Glib::RefPtr<Glib::KeyFile> m_file;

    std::vector<ELCloneRepo> parseData();
    void unparseData(const std::vector<ELCloneRepo>& data);

    std::vector<ELCloneRepo> m_repositories;

public:
    explicit ELRepoFile(const std::filesystem::path& path)
        : GenericFile(path)
    {
    }

    virtual void read() override;
    virtual void write() override;

    void parse();
    void parse(const std::stringstream& ss);
    void unparse();
    void unparse(std::stringstream& ss);

    [[nodiscard]] std::vector<ELCloneRepo>& getRepositories();
    [[nodiscard]] const std::vector<ELCloneRepo>& getRepositoriesConst() const;

    ~ELRepoFile() override = default;
};

#endif
