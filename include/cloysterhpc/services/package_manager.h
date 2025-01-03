/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PACKAGE_MANAGER_H_
#define CLOYSTERHPC_PACKAGE_MANAGER_H_

#include <list>
#include <string>
#include <string_view>

class package_manager {
public:
    virtual ~package_manager() = default;

    package_manager(const package_manager&) = delete;
    package_manager& operator=(const package_manager&) = delete;

    package_manager(package_manager&&) = delete;
    package_manager& operator=(package_manager&&) = delete;

    virtual bool install(std::string_view package) = 0;
    virtual bool remove(std::string_view package) = 0;
    virtual bool update(std::string_view package) = 0;
    virtual bool update() = 0;
    virtual void check() = 0;
    virtual void clean() = 0;
    virtual std::list<std::string> repolist() = 0;
};

#endif // CLOYSTERHPC_PACKAGE_MANAGER_H_
