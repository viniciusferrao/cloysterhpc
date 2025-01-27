/*
 * Copyright 2025 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

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

Glib::RefPtr<Glib::KeyFile> ELRepoFile::loadFile()
{
    try {
        auto file = Glib::KeyFile::create();
        file->load_from_file(m_path.string());
        return file;
    } catch (Glib::FileError& e) {
        throw repository_exception(
            std::format("Could not load repository file {} ({})",
                m_path.string(), e.what()));
    }
}

void ELRepoFile::read()
{
    printf("reading\n");

    auto file = loadFile();

    auto reponames = file->get_groups();

    for (const auto& repogroup : reponames) {
        auto name = file->get_string(repogroup, "name");

        if (name.empty()) {
            throw repository_exception(std::format(
                "Could not load repo name from repo '{}' at file {}",
                repogroup.raw(), m_path.string()));
        }

        auto metalink = (file->has_key(repogroup, "metalink"))
            ? std::make_optional(file->get_string(repogroup, "metalink"))
            : std::nullopt;
        auto baseurl = (file->has_key(repogroup, "baseurl"))
            ? std::make_optional(file->get_string(repogroup, "baseurl"))
            : std::nullopt;

        auto enabled = file->get_boolean(repogroup, "enabled");
        auto gpgcheck = file->get_boolean(repogroup, "gpgcheck");
        auto gpgkey = file->get_string(repogroup, "gpgkey");

        ELRepo repo;
        repo.group = repogroup.raw();
        repo.name = name.raw();
        repo.metalink
            = metalink.transform([](const auto& v) { return v.raw(); });
        repo.baseURL = baseurl.transform([](const auto& v) { return v.raw(); });
        repo.enabled = enabled;
        repo.gpgcheck = gpgcheck;
        repo.gpgkey = gpgkey;
        m_repositories.push_back(std::move(repo));
    }

    for (const auto& r : m_repositories) {
        std::print("repo found '{}'\n", r.group);
        std::print("\tname: '{}'\n", r.name);
        std::print("\tbaseurl: '{}'\n", r.baseURL.value_or("null"));
        std::print("\tmetalink: '{}'\n", r.metalink.value_or("null"));
        std::print("\tenabled: {}\n", r.enabled);
        std::print("\tgpg: (check: {}, key: {})\n", r.gpgcheck, r.gpgkey);
    }
}

void ELRepoFile::write()
{
    auto file = loadFile();

    for (const auto& repo : m_repositories) {
        file->set_string(repo.group, "name", repo.name);
        file->set_boolean(repo.group, "enabled", repo.enabled);
        file->set_boolean(repo.group, "gpgcheck", repo.gpgcheck);
        file->set_string(repo.group, "gpgkey", repo.gpgkey);
    }

    file->save_to_file(m_path.string());
}
