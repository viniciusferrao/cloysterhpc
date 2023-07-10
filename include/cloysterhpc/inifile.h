//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 7/6/23.
//

#ifndef CLOYSTERHPC_INIFILE_H_
#define CLOYSTERHPC_INIFILE_H_

#include "../../src/include/SimpleIni.h"
#include <filesystem>

class inifile {
private:
    CSimpleIniA ini;

public:
    void loadFile(const std::string& filepath);
    void loadFile(const std::string_view& filepath);
    void loadFile(const std::filesystem::path& filepath);
    std::string getValue(const std::string& section, const std::string& key);
    void setValue(const std::string& section, const std::string& key,
        const std::string& newValue);
    bool deleteValue(const std::string& section, const std::string& key);
    void saveFile(const std::string& filepath);
    void saveFile(const std::string_view& filepath);
    void saveFile(const std::filesystem::path& filepath);
    bool exists(const std::string& section, const std::string& key);
    bool exists(const std::string& section);
    inifile();
};

#endif // CLOYSTERHPC_INIFILE_H_
