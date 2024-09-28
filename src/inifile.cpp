/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cloysterhpc/inifile.h>
#include <filesystem>
#include <fmt/format.h>

void inifile::loadData(const std::string& data) { ini.LoadData(data); }

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
    SI_Error result = ini.LoadFile(filepath.c_str());
    if (result < 0) {
        throw std::runtime_error(
            fmt::format(R"(File "{}" could not be open, error {} )",
                filepath.c_str(), result));
    }
}

std::string inifile::getValue(const std::string& section,
    const std::string& key, const bool optional, const bool canBeNull)
{
    if (!optional && !exists(section, key))
        throw std::runtime_error(
            fmt::format(R"(Answerfile section "{}" must have "{}" key filled)",
                section, key));

    std::string value = ini.GetValue(section.c_str(), key.c_str(), "");

    if (!canBeNull && value.empty())
        throw std::runtime_error(fmt::format(
            R"(Answerfile section "{}" key "{}" can't be null)", section, key));

    return value;
}

inifile::inifile() { ini.SetUnicode(); }

inifile::inifile(const std::string& file)
{
    ini.SetUnicode();
    loadData(file);
}

inifile inifile::mergeInto(const std::filesystem::path& other)
{
    inifile otherfile(other);

    for (const auto& sec : this->listAllSections()) {
        for (const auto& entry : this->listAllEntries(sec)) {
            const auto thisval = this->getValue(sec, entry);
            otherfile.setValue(sec, entry, thisval);
        }
    }

    /* Since the CSimpleIniA class we encapsulate, for some reason, does not
     * have a copy constructor, we need to save the object into a string
     * (that can be copied somewhat) and recreate the object before returning
     */
    std::string data;
    otherfile.save(data);

    return inifile { data };
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
    return ini.KeyExists(section.c_str(), key.c_str());
}

// BUG: Returning a pointer is not a good idea, it causes ownership issues.
bool inifile::exists(const std::string& section)
{
    return ini.SectionExists(section.c_str());
}

static std::vector<std::string> convertIniNames(
    CSimpleIniA::TNamesDepend&& names)
{
    std::vector<std::string> ret;

    std::transform(names.begin(), names.end(), std::back_inserter(ret),
        [](auto entry) { return std::string { entry.pItem }; });

    return ret;
}

std::vector<std::string> inifile::listAllSections() const
{
    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);

    return convertIniNames(std::move(sections));
}

std::vector<std::string> inifile::listAllEntries(
    const std::string& section) const
{
    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(section.c_str(), keys);

    return convertIniNames(std::move(keys));
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
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        ini.loadFile(path);
        const std::string clusterName
            = ini.getValue("information", "cluster_name");
        CHECK(clusterName == "cloyster");
    }

    TEST_CASE("Get sections")
    {
        inifile ini;
        std::filesystem::path path;
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        ini.loadFile(path);

        auto ret = ini.listAllSections();
        REQUIRE(ret.size() == 2);
        std::sort(ret.begin(), ret.end());
        CHECK(ret[0] == "another");
        CHECK(ret[1] == "information");
    }

    TEST_CASE("Get section entries")
    {
        inifile ini;
        std::filesystem::path path;
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        ini.loadFile(path);

        auto ret = ini.listAllEntries("another");
        REQUIRE(ret.size() == 2);
        std::sort(ret.begin(), ret.end());
        CHECK(ret[0] == "another_key");
        CHECK(ret[1] == "second_key");
    }

    TEST_CASE("Set value")
    {
        inifile ini;
        std::filesystem::path path;
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        ini.loadFile(path);
        const std::string newValue = "modified";
        ini.setValue("information", "cluster_name", newValue);
        CHECK(ini.getValue("information", "cluster_name") == newValue);
    }

    TEST_CASE("Delete value")
    {
        inifile ini;
        std::filesystem::path path;
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        ini.loadFile(path);
        const bool result = ini.deleteValue("information", "company_name");
        CHECK(result);
    }

    TEST_CASE("Save to a new file")
    {
        inifile ini;
        std::filesystem::path path;
        std::filesystem::path newFile;
        path = TEST_SAMPLE_DIR / std::filesystem::path { "inifile.ini" };
        newFile = std::filesystem::current_path() / "newinifile.ini";
        ini.loadFile(path);
        ini.saveFile(newFile);
        CHECK(std::filesystem::exists(newFile));
    }
}
