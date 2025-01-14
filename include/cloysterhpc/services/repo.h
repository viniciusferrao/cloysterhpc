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

#endif // CLOYSTERHPC_REPO_H_
