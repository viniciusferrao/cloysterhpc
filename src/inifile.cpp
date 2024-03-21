/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/inifile.h>
#include <fmt/format.h>

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

void inifile::setValue(const std::string& section, const std::string& key,
    const std::string& newValue)
{
    ini.SetValue(section.c_str(), key.c_str(), newValue.c_str());
}

bool inifile::deleteValue(const std::string& section, const std::string& key)
{
    return ini.Delete(section.c_str(), key.c_str());
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
#include <cloysterhpc/tests.h>

TEST_SUITE("Test Inifile")
{
    TEST_CASE("Test Inifile generic methods ")
    {
        SUBCASE("Get information")
        {
            inifile ini { tests::sampleDirectory / "inifile.ini" };
            const std::string clusterName
                = ini.getValue("information", "cluster_name");
            CHECK((clusterName == "cloyster"));
            MESSAGE("Cluster name: ", clusterName);
        }

        SUBCASE("Set value")
        {
            inifile ini { tests::sampleDirectory / "inifile.ini" };
            const std::string newValue = "modified";
            ini.setValue("information", "cluster_name", newValue);

            const std::string clusterName
                = ini.getValue("information", "cluster_name");
            CHECK((clusterName == newValue));
            MESSAGE("Modified cluster name: ", clusterName);
        }

        SUBCASE("Delete value")
        {
            inifile ini { tests::sampleDirectory / "inifile.ini" };
            const bool result = ini.deleteValue("information", "company_name");
            CHECK(result);
        }

        SUBCASE("Save to a new file")
        {
            inifile ini { tests::sampleDirectory / "inifile.ini" };
            auto newFile = tests::sampleDirectory / "newfile.ini";
            ini.saveFile(newFile);
            CHECK(std::filesystem::exists(newFile));
            std::filesystem::remove(newFile);
        }
    }
}
#endif
