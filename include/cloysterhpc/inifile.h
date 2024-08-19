/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_INIFILE_H_
#define CLOYSTERHPC_INIFILE_H_

#include <SimpleIni.h>
#include <filesystem>

/**
 * @class inifile
 * @brief A class for handling INI file operations.
 *
 * This class provides methods to load, save, get, set, and delete values
 * from INI files using the CSimpleIniA library.
 */
class inifile {
private:
    CSimpleIniA ini;

public:
    /**
     * @brief Loads an INI file from the specified file path.
     *
     * @param filepath The path to the INI file.
     */
    void loadFile(const std::string& filepath);

    /**
     * @brief Loads an INI file from the specified file path.
     *
     * @param filepath The path to the INI file.
     */
    void loadFile(std::string_view filepath);

    /**
     * @brief Loads an INI file from the specified file path.
     *
     * @param filepath The path to the INI file.
     */
    void loadFile(const std::filesystem::path& filepath);
    void loadData(const std::string& data);
    /**
     * @brief Retrieves a value from the INI file.
     *
     * @param section The section in the INI file.
     * @param key The key in the section.
     * @param optional If true, the method will not throw an error if the key
     * does not exist.
     * @param canBeNull If true, the method will return an empty string if the
     * key does not exist.
     * @return The value associated with the specified key in the section.
     */
    std::string getValue(const std::string& section, const std::string& key,
        const bool optional = true, const bool canBeNull = true);

    /**
     * @brief Sets a value in the INI file.
     *
     * @param section The section in the INI file.
     * @param key The key in the section.
     * @param newValue The new value to set for the specified key.
     */
    void setValue(const std::string& section, const std::string& key,
        const std::string& newValue);

    /**
     * @brief Deletes a value from the INI file.
     *
     * @param section The section in the INI file.
     * @param key The key in the section.
     * @return True if the value was successfully deleted, false otherwise.
     */
    bool deleteValue(const std::string& section, const std::string& key);
    void save(std::string& holder) const;

    /**
     * @brief Saves the INI file to the specified file path.
     *
     * @param filepath The path to save the INI file.
     */
    void saveFile(const std::string& filepath);

    /**
     * @brief Saves the INI file to the specified file path.
     *
     * @param filepath The path to save the INI file.
     */
    void saveFile(std::string_view filepath);

    /**
     * @brief Saves the INI file to the specified file path.
     *
     * @param filepath The path to save the INI file.
     */
    void saveFile(const std::filesystem::path& filepath);

    /**
     * @brief Checks if a key exists in a section of the INI file.
     *
     * @param section The section in the INI file.
     * @param key The key to check for existence.
     * @return True if the key exists in the section, false otherwise.
     */
    bool exists(const std::string& section, const std::string& key);

    /**
     * @brief Checks if a section exists in the INI file.
     *
     * @param section The section to check for existence.
     * @return True if the section exists, false otherwise.
     */
    bool exists(const std::string& section);
    inifile(const std::string& file);
    inifile();
};

#endif // CLOYSTERHPC_INIFILE_H_
