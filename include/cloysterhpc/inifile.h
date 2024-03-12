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
    // https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
    template <typename FilePath> void loadFile(FilePath&& path)
    {
        if constexpr (std::is_convertible_v<FilePath, std::string>
            || std::is_same_v<FilePath, std::filesystem::path>) {
            ini.LoadFile(std::forward<FilePath>(path).c_str());
        } else if constexpr (std::is_convertible_v<FilePath,
                                 std::string_view>) {
            ini.LoadFile(std::forward<FilePath>(path).data());
        } else {
            throw std::invalid_argument("Unsupported inifile path object type");
        }
    }

    std::string getValue(const std::string& section, const std::string& key,
        const bool optional = true, const bool canBeNull = true);
    void setValue(const std::string& section, const std::string& key,
        const std::string& newValue);
    bool deleteValue(const std::string& section, const std::string& key);
    void saveFile(const std::string& filepath);
    void saveFile(std::string_view filepath);
    void saveFile(const std::filesystem::path& filepath);
    bool exists(const std::string& section, const std::string& key);
    bool exists(const std::string& section);
    inifile();
};

#endif // CLOYSTERHPC_INIFILE_H_
