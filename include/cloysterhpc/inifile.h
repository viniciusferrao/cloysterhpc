/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_INIFILE_H_
#define CLOYSTERHPC_INIFILE_H_

#include "functions.h"
#include <SimpleIni.h>
#include <filesystem>

class inifile {
private:
    CSimpleIniA ini;

public:
    template <typename FilePath> void saveFile(FilePath&& path)
    {
        ini.SaveFile(
            cloyster::handlePath(std::forward<FilePath>(path)).c_str());
    }

    template <typename FilePath> void loadFile(FilePath&& path)
    {
        ini.LoadFile(
            cloyster::handlePath(std::forward<FilePath>(path)).c_str());
    }

    std::string getValue(const std::string& section, const std::string& key,
        const bool optional = true, const bool canBeNull = true);
    void setValue(const std::string& section, const std::string& key,
        const std::string& newValue);
    bool deleteValue(const std::string& section, const std::string& key);
    bool exists(const std::string& section, const std::string& key);
    bool exists(const std::string& section);
    inifile();
    template <typename FilePath> explicit inifile(FilePath&& path)
    {
        ini.SetUnicode();
        loadFile(std::forward<FilePath>(path));
    }
};

#endif // CLOYSTERHPC_INIFILE_H_
