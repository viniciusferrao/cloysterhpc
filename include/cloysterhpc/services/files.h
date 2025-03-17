#ifndef CLOYSTER_SERVICES_FILES_H
#define CLOYSTER_SERVICES_FILES_H

#include <concepts>
#include <filesystem>
#include <string>
#include <vector>

#include <cloysterhpc/concepts.h>

namespace cloyster::services::files {

template <typename File>
concept IsKeyFileReadable = requires(
    const File& file, const std::string& group, const std::string& key) {
    { file.getGroups() } -> std::convertible_to<std::vector<std::string>>;
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
    explicit KeyFile(Impl&& impl);

public:
    ~KeyFile(); // The destructor is required to be defined in
                // the .cpp file where the size of Impl is known

    // Moveable, not Copiable
    KeyFile(const KeyFile&) = delete;
    KeyFile(KeyFile&& file) = default;
    KeyFile& operator=(const KeyFile&) = delete;
    KeyFile& operator=(KeyFile&&) = default;

    [[nodiscard]] std::vector<std::string> getGroups() const;
    [[nodiscard]] std::string getString(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] bool getBoolean(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] std::optional<std::string> getStringOpt(
        const std::string& group, const std::string& key) const;
    [[nodiscard]] std::string toData() const;

    void setString(const std::string& group, const std::string& key,
        const std::string& value);
    void setString(const std::string& group, const std::string& key,
        const std::optional<std::string>& value);
    template <typename Stringable>
        requires std::convertible_to<Stringable, std::string>
    void setString(const Stringable& group, const Stringable& key,
        const Stringable& value);
    void setBoolean(
        const std::string& group, const std::string& key, const bool value);

    void save();
    void load();

    explicit KeyFile(const std::filesystem::path& path);
    // explicit KeyFile(const std::string& str);
};
static_assert(IsKeyFileReadable<KeyFile>);
static_assert(IsKeyFileWriteable<KeyFile>);
static_assert(cloyster::concepts::IsSaveable<KeyFile>);
static_assert(concepts::IsMoveable<KeyFile>);
static_assert(!concepts::IsCopyable<KeyFile>);


std::string checksum(const std::string& data);
std::string checksum(const std::filesystem::path& path, const std::size_t chunkSize = 16384);

};

#endif
