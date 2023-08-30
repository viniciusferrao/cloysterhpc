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

std::string inifile::getValue(
    const std::string& section, const std::string& key, const bool& optional)
{
    if (!optional && !exists(section, key))
        throw std::runtime_error(
            fmt::format("Answerfile section \"{}\" must have \"{}\" key filled",
                section, key));

    return ini.GetValue(section.c_str(), key.c_str(), "");
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

void inifile::saveFile(std::string_view filepath)
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
