/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/inifile.h>
#include <fmt/format.h>

// TODO: Template<T> the next three functions
void inifile::loadFile(const std::string& filepath)
{
    ini.LoadFile(filepath.c_str());
}

void inifile::loadFile(std::string_view filepath)
{
    ini.LoadFile(filepath.data());
}

void inifile::loadFile(const std::filesystem::path& filepath)
{
    ini.LoadFile(filepath.c_str());
}

void inifile::loadData(const std::string& data) { ini.LoadData(data); }

std::string inifile::getValue(const std::string& section,
    const std::string& key, const bool optional, const bool canBeNull)
{
    if (!optional && !exists(section, key))
        throw std::runtime_error(
            fmt::format("Answerfile section \"{}\" must have \"{}\" key filled",
                section, key));

    std::string value = ini.GetValue(section.c_str(), key.c_str(), "");

    if (!canBeNull && value.empty())
        throw std::runtime_error(
            fmt::format("Answerfile section \"{}\" key \"{}\" can't be null",
                section, key));

    return value;
}

inifile::inifile() { ini.SetUnicode(); }

inifile::inifile(const std::string& file)
{
    ini.SetUnicode();
    loadData(file);
}

void inifile::setValue(const std::string& section, const std::string& key,
    const std::string& newValue)
{
    ini.SetValue(section.c_str(), key.c_str(), newValue.c_str());
}

bool inifile::deleteValue(const std::string& section, const std::string& key)
{
    return ini.Delete(section.c_str(), key.c_str());
}

// Saves inifile data in a std::string
void inifile::save(std::string& holder) const { ini.Save(holder); }

void inifile::saveFile(const std::string& filepath)
{
    ini.SaveFile(filepath.c_str());
}

void inifile::saveFile(std::string_view filepath)
{
    ini.SaveFile(filepath.data());
}

void inifile::saveFile(const std::filesystem::path& filepath)
{
    ini.SaveFile(filepath.c_str());
}

// BUG: Returning a pointer is not a good idea, it causes ownership issues.
bool inifile::exists(const std::string& section, const std::string& key)
{
    const auto* sample = ini.GetValue(section.c_str(), key.c_str());
    return sample != nullptr;
}

// BUG: Returning a pointer is not a good idea, it causes ownership issues.
bool inifile::exists(const std::string& section)
{
    const auto* sample = ini.GetSection(section.c_str());
    return sample != nullptr;
}

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

TEST_SUITE("Load .ini files")
{

    TEST_CASE("Get information")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        const std::string clusterName
            = ini.getValue("information", "cluster_name");
        CHECK(clusterName == "cloyster");
    }

    TEST_CASE("Set value")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        const std::string newValue = "modified";
        ini.setValue("information", "cluster_name", newValue);
        CHECK(ini.getValue("information", "cluster_name") == newValue);
    }

    TEST_CASE("Delete value")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        const bool result = ini.deleteValue("information", "company_name");
        CHECK(result);
    }

    TEST_CASE("Save to a new file")
    {
        inifile ini;
        std::filesystem::path path;
        std::filesystem::path newFile;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        newFile = std::filesystem::current_path() / "sample/newinifile.ini";
        ini.loadFile(path);
        ini.saveFile(newFile);
        CHECK(std::filesystem::exists(newFile));
    }
}
