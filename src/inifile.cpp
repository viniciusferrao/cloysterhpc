//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 7/6/23.
//

#include "cloysterhpc/inifile.h"

void inifile::loadFile(const std::string& filepath)
{
    ini.LoadFile(filepath.c_str());
}

void inifile::loadFile(const std::string_view& filepath)
{
    ini.LoadFile(filepath.data());
}

void inifile::loadFile(const std::filesystem::path& filepath)
{
    ini.LoadFile(filepath.c_str());
}

std::string inifile::getValue(
    const std::string& section, const std::string& key)
{
    return ini.GetValue(section.c_str(), key.c_str());
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
void inifile::saveFile(const std::string& filepath)
{
    ini.SaveFile(filepath.c_str());
}

void inifile::saveFile(const std::string_view& filepath)
{
    ini.SaveFile(filepath.data());
}

void inifile::saveFile(const std::filesystem::path& filepath)
{
    ini.SaveFile(filepath.c_str());
}

bool inifile::exists(const std::string& section, const std::string& key)
{
    const auto* sample = ini.GetValue(section.c_str(), key.c_str());
    return sample != nullptr;
}

bool inifile::exists(const std::string& section)
{
    const auto* sample = ini.GetSection(section.c_str());
    return sample != nullptr;
}
