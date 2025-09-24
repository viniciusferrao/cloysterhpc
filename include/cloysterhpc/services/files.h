#ifndef CLOYSTER_SERVICES_FILES_H
#define CLOYSTER_SERVICES_FILES_H

#include <concepts>
#include <filesystem>
#include <string>
#include <vector>

#include <cloysterhpc/concepts.h>

namespace cloyster::services::files {

constexpr std::size_t CHUNK_SIZE = 16384;

template <typename File>
concept IsKeyFileReadable = requires(
    const File& file, const std::string& group, const std::string& key) {
    { file.getGroups() } -> std::same_as<std::vector<std::string>>;
    { file.getString(group, key) } -> std::same_as<std::string>;
    { file.getBoolean(group, key) } -> std::same_as<bool>;
    {
        file.getStringOpt(group, key)
    } -> std::same_as<std::optional<std::string>>;
};

template <typename File>
concept IsKeyFileWriteable = requires(File& file, const std::string& group,
    const std::string& key, bool bvalue, const std::string& svalue,
    const std::optional<std::string>& soptvalue) {
    file.setString(group, key, svalue);
    file.setString(group, key, soptvalue);
    file.setBoolean(group, key, bvalue);
};

/**
 * @brief Wraps implementation exceptions
 */
class FileException : public std::runtime_error {
public:
    explicit FileException(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
};

/**
 * @brief Represents a KeyFile hiding the implementation details behind Impl
 */
class KeyFile {
    struct Impl; // Pointer to Implementation (PIMPL) pattern
    std::unique_ptr<Impl> m_impl;

public:
    ~KeyFile(); // The destructor is required to be defined in
                // the .cpp file where the size of Impl is known

    // Moveable, not Copiable
    KeyFile(const KeyFile&) = delete;
    KeyFile(KeyFile&& file) = default;
    KeyFile& operator=(const KeyFile&) = delete;
    KeyFile& operator=(KeyFile&&) = default;

    [[nodiscard]] std::vector<std::string> listAllPrefixedEntries(
        const std::string_view prefix) const;
    [[nodiscard]] bool hasGroup(std::string_view group) const;
    [[nodiscard]] std::vector<std::string> getGroups() const;
    [[nodiscard]] std::string getString(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] std::string getString(const std::string& group,
        const std::string& key, std::string&& defaultValue) const;
    [[nodiscard]] bool getBoolean(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] std::optional<std::string> getStringOpt(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] std::string toData() const;

    void setString(const std::string& group, const std::string& key,
        const std::string& value);
    void setString(const std::string& group, const std::string& key,
        const std::optional<std::string>& value);
    void setBoolean(
        const std::string& group, const std::string& key, const bool value);

    void save();
    void load();
    void loadData(const std::string& data);

    explicit KeyFile(const std::filesystem::path& path);
};
static_assert(IsKeyFileReadable<KeyFile>);
static_assert(IsKeyFileWriteable<KeyFile>);
static_assert(cloyster::concepts::IsSaveable<KeyFile>);
static_assert(concepts::IsMoveable<KeyFile>);
static_assert(!concepts::IsCopyable<KeyFile>);

std::string checksum(const std::string& data);
std::string checksum(const std::filesystem::path& path,
    const std::size_t chunkSize = CHUNK_SIZE);

std::string md5sum(const std::string& data);

namespace fs = std::filesystem;

/**
 * @brief Checks if a file or directory exists.
 *
 * @param path The filesystem path to check.
 * @return true if the path exists, false otherwise.
 */
bool exists(const fs::path& path);

/**
 * @brief Creates an empty file at the specified path.
 *
 * If the file already exists, this function does nothing.
 * Ensures that all parent directories exist.
 *
 * @param path The filesystem path of the file to create.
 * @throws std::system_error if the file cannot be created.
 */
void create(const fs::path& path);

/**
 * @brief Removes the file at the specified path.
 *
 * If the file does not exist, this function does nothing.
 *
 * @param path The filesystem path of the file to remove.
 * @throws std::system_error if the file cannot be removed.
 */
void remove(const fs::path& path);

/**
 * @brief Reads the entire contents of a file into a string.
 *
 * @param path The filesystem path of the file to read.
 * @return A string containing the full contents of the file.
 * @throws std::system_error if the file cannot be opened or read.
 */
std::string read(const fs::path& path);

/**
 * @brief Writes the given contents to a file, overwriting any existing content.
 *
 * @param path The filesystem path of the file to write.
 * @param contents The data to write into the file.
 * @throws std::system_error if the file cannot be opened or written.
 */
void write(const fs::path& path, std::string_view contents);

/**
 * @brief Appends the given contents to the end of a file.
 *
 * If the file does not exist, it will be created.
 *
 * @param path The filesystem path of the file to append to.
 * @param contents The data to append to the file.
 * @throws std::system_error if the file cannot be opened or written.
 */
void append(const fs::path& path, std::string_view contents);

};

#endif
