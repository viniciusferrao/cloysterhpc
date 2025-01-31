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

void ELRepoFile::read()
{
    try {
        m_file = Glib::KeyFile::create();
        m_file->load_from_file(m_path.string());
    } catch (Glib::FileError& e) {
        throw repository_exception(
            std::format("Could not load repository file {} ({})",
                m_path.string(), e.what()));
    }
}

void ELRepoFile::write() { m_file->save_to_file(m_path.string()); }

std::vector<ELRepo> ELRepoFile::parse()
{
    read();

    return parseData();
}

std::vector<ELRepo> ELRepoFile::parse(const std::stringstream& ss)
{
    m_file = Glib::KeyFile::create();
    m_file->load_from_data(ss.str().c_str());
    return parseData();
}

std::vector<ELRepo> ELRepoFile::parseData()
{
    auto reponames = m_file->get_groups();

    std::vector<ELRepo> repositories;

    for (const auto& repogroup : reponames) {
        auto name = m_file->get_string(repogroup, "name");

        if (name.empty()) {
            throw repository_exception(std::format(
                "Could not load repo name from repo '{}' at m_file {}",
                repogroup.raw(), m_path.string()));
        }

        auto metalink = cloyster::readKeyfileString(
            m_file, std::string_view { repogroup.c_str() }, "metalink");
        auto baseurl = cloyster::readKeyfileString(
            m_file, std::string_view { repogroup.c_str() }, "baseurl");

        auto enabled = m_file->get_boolean(repogroup, "enabled");
        auto gpgcheck = m_file->get_boolean(repogroup, "gpgcheck");
        auto gpgkey = m_file->get_string(repogroup, "gpgkey");

        ELRepo repo;
        repo.group = repogroup.raw();
        repo.name = name.raw();
        repo.metalink
            = metalink.transform([](const auto& v) { return v.raw(); });
        repo.base_url
            = baseurl.transform([](const auto& v) { return v.raw(); });
        repo.enabled = enabled;
        repo.gpgcheck = gpgcheck;
        repo.gpgkey = gpgkey;
        repositories.push_back(std::move(repo));
    }

    return repositories;
}

void ELRepoFile::unparseData(const std::vector<ELRepo>& repositories)
{
    for (const auto& repo : repositories) {
        m_file->set_string(repo.group, "name", repo.name);
        m_file->set_boolean(repo.group, "enabled", repo.enabled);
        m_file->set_boolean(repo.group, "gpgcheck", repo.gpgcheck);
        m_file->set_string(repo.group, "gpgkey", repo.gpgkey);
    }
}

void ELRepoFile::unparse(const std::vector<ELRepo>& repositories)
{
    unparseData(repositories);
    write();
}

void ELRepoFile::unparse(
    const std::vector<ELRepo>& repositories, std::stringstream& ss)
{
    unparseData(repositories);
    ss.seekp(0);
    ss << m_file->to_data();
}
