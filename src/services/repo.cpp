/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/repo.h>
#include <cloysterhpc/services/log.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>
#include <unordered_map>

// TEST CODE: Load a repository file and dump it on the terminal
// Call it in main():
// repo::load_repository(std::string { "/etc/yum.repos.d/redhat.repo" });

