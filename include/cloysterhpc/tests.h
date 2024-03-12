/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_TESTS_H_
#define CLOYSTERHPC_TESTS_H_

#include <doctest/doctest.h>
#include <filesystem>

namespace tests {
inline std::filesystem::path sampleDirectory {
    std::filesystem::current_path().parent_path().parent_path() / "test/sample"
}; // current_path corresponds to '<cloyster_main_dir>/<build_dir>/test'
}

#endif // CLOYSTERHPC_TESTS_H_
