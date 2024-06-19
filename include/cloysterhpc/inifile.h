/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_INIFILE_H_
#define CLOYSTERHPC_INIFILE_H_

#include <SimpleIni.h>
#include <filesystem>

class inifile {
private:
    CSimpleIniA ini;

public:
    void loadFile(const std::string& filepath);
    void loadFile(std::string_view filepath);
    void loadFile(const std::filesystem::path& filepath);
    void loadData(const std::string& data);
    std::string getValue(const std::string& section, const std::string& key,
        const bool optional = true, const bool canBeNull = true);
    void setValue(const std::string& section, const std::string& key,
        const std::string& newValue);
    bool deleteValue(const std::string& section, const std::string& key);
    void save(std::string& holder) const;
    void saveFile(const std::string& filepath);
    void saveFile(std::string_view filepath);
    void saveFile(const std::filesystem::path& filepath);
    bool exists(const std::string& section, const std::string& key);
    bool exists(const std::string& section);
    inifile(const std::string& file);
    inifile();
};

#endif // CLOYSTERHPC_INIFILE_H_
