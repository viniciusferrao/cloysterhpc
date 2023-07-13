/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../src/inifile.h"
#include "../../src/inifile.cpp"
#include <doctest/doctest.h>
#include <filesystem>
#include <iostream>

TEST_SUITE("Load files")
{

    TEST_CASE("Get information")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        std::string clusterName = ini.getValue("information", "cluster_name");
        CHECK(clusterName == "cloyster");
    }

    TEST_CASE("Set value")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        std::string newValue = "modified";
        ini.setValue("information", "cluster_name", newValue);
        CHECK(ini.getValue("information", "cluster_name") == newValue);
    }

    TEST_CASE("Delete value")
    {
        inifile ini;
        std::filesystem::path path;
        path = std::filesystem::current_path() / "sample/inifile.ini";
        ini.loadFile(path);
        bool result = ini.deleteValue("information", "company_name");
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
