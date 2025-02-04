/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/repo.h>
#include <fmt/printf.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

// TEST CODE: Load a repository file and dump it on the terminal
// Call it in main():
// repo::load_repository(std::string { "/etc/yum.repos.d/redhat.repo" });
void repo::load_repository(std::filesystem::path path)
{
    try {
        auto keyfile = Glib::KeyFile::create();
        keyfile->load_from_file(path.string());

        auto groups = keyfile->get_groups();

        fmt::printf("Reading .repo file: {}\n", path.string());
        fmt::printf("Found {} group(s):\n", groups.size());

        for (const auto& group : groups) {
            fmt::printf("\n[{}]\n", group.c_str());

            // For each group, get all keys
            auto keys = keyfile->get_keys(group);

            // Print each key=value
            for (const auto& key : keys) {
                // You can use get_value() or get_string() depending on your
                // needs
                auto value = keyfile->get_value(group, key);
                fmt::print("  {} = {}\n", key.c_str(), value.c_str());
            }
        }

        /*
        if (keyfile->has_group("my-new-repo")) {
            keyfile->set_string("my-new-repo", "enabled", "1");
        } else {
            keyfile->set_string("my-new-repo", "name", "My New Repo");
            keyfile->set_string(
                "my-new-repo", "baseurl", "https://example.com/repo");
            keyfile->set_string("my-new-repo", "enabled", "1");
        }

        keyfile->save_to_file(path.string());
        */

    } catch (const Glib::FileError& ex) {
        fmt::print("File Error: {}\n", ex.what());
    } catch (const Glib::KeyFileError& ex) {
        fmt::print("KeyFile Error: {}\n", ex.what());
    }
}
