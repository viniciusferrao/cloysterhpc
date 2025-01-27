/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_REPO_H_
#define CLOYSTERHPC_REPO_H_

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

/**
 * @brief This class represents an EL .repo file as it's found in
 * /etc/yum.repos.d/
 * @note This class is WIP, it was added as a PoC of glibmm integration
 */
class repo {

public:
    std::string_view m_name {};
    std::string_view m_description();
    bool m_enabled = false;
    std::string_view m_baseurl {};
    struct m_gpg {
        bool check = false;
        std::string_view key {};
    };
    struct m_ssl {
        bool verify = false;
        std::string_view cacert {};
        std::string_view clientkey {};
        std::string_view clientcert {};
        bool verify_status;
    };
    std::uint32_t m_metadata_expire = 86400;
    bool m_metadata_enabled = false;

    std::vector<std::string_view> m_include_packages;
    std::vector<std::string_view> m_exclude_packages;

    std::string_view name() const;
    void name(std::string_view name);

    static void load_repository(std::filesystem::path path);
};

class repository_exception : public std::runtime_error {
public:
    explicit repository_exception(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
};

/**
 * Repository file class
 */
template <class Repo> class RepoFile {
protected:
    std::filesystem::path m_path;
    std::vector<Repo> m_repositories;

public:
    RepoFile(const std::filesystem::path& path)
        : m_path(path)
    {
    }

    virtual void read() { }
    virtual void write() { }
};

struct ELRepo {
    std::string group;
    std::string name;
    std::optional<std::string> baseURL;
    std::optional<std::string> metalink;
    bool enabled;
    bool gpgcheck;
    std::string gpgkey;

    // (P/ todos os repositórios)
    // std::string release;
};

/**
 * Repository file class
 */
class ELRepoFile : public RepoFile<ELRepo> {
private:
    Glib::RefPtr<Glib::KeyFile> loadFile();

public:
    ELRepoFile(const std::filesystem::path& path)
        : RepoFile(path)
    {
    }

    void read() override;
    void write() override;
};

#endif // CLOYSTERHPC_REPO_H_
