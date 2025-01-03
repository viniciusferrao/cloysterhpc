/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_DNF_H_
#define CLOYSTERHPC_DNF_H_

#include <cloysterhpc/services/package_manager.h>

class dnf : public package_manager {
public:
    bool install(std::string_view package) override;
    bool remove(std::string_view package) override;
    bool update(std::string_view package) override;
    bool update() override;
    void check() override;
    void clean() override;
    std::list<std::string> repolist() override;
};

#endif // CLOYSTERHPC_DNF_H_
