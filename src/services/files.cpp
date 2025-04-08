#include <cstddef>
#include <fstream>
#include <ios>
#include <istream>
#include <ranges>
#include <stdexcept>
#include <utility>

#include <glibmm/checksum.h>
#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/functions.h>

namespace cloyster::services::files {

struct KeyFile::Impl {
    std::filesystem::path m_path;
    std::unique_ptr<Glib::KeyFile> m_keyfile;

    Impl(Glib::KeyFile&& keyfile, std::filesystem::path path)
        : m_path(std::move(path))
        , m_keyfile(std::make_unique<Glib::KeyFile>(std::move(keyfile))) { };

};

KeyFile::KeyFile(const std::filesystem::path& path)
    : m_impl(std::make_unique<KeyFile::Impl>(Glib::KeyFile(), path))
{
    m_impl->m_path = path;
    if (cloyster::exists(path)) {
        m_impl->m_keyfile->load_from_file(path);
    }
}

KeyFile::~KeyFile() = default;

namespace {
    std::vector<std::string> toStrings(const std::vector<Glib::ustring>& input)
    {
        return input | std::views::transform([](const auto& group) {
            return group.raw();
        }) | std::ranges::to<std::vector<std::string>>();
    }
}

std::vector<std::string> KeyFile::listAllPrefixedEntries(
    const std::string_view prefix) const
{
    auto groups = getGroups();

    return groups | std::views::filter([&](const auto& group) {
        return group.starts_with(prefix);
    }) | std::ranges::to<std::vector>();
}

std::vector<std::string> KeyFile::getGroups() const
{
    return toStrings(m_impl->m_keyfile->get_groups());
}

bool KeyFile::hasGroup(std::string_view group) const
{
    return m_impl->m_keyfile->has_group(std::string(group));
}

std::string KeyFile::getString(
    const std::string& group, const std::string& key) const
{
    try {
        return m_impl->m_keyfile->get_string(group, key).raw();
    } catch (Glib::KeyFileError& e) {
        throw std::runtime_error(
            fmt::format("Keyfile Error, no such entry {} {}", group, key));
    }
}

std::string KeyFile::getString(
    const std::string& group, const std::string& key, std::string&& defaultValue) const
{
    try {
        if (!m_impl->m_keyfile->has_key(group, key)) {
            return std::move(defaultValue);
        }
        return m_impl->m_keyfile->get_string(group, key).raw();
    } catch (Glib::KeyFileError& e) {
        throw std::runtime_error(
            fmt::format("Keyfile Error, no such entry {} {}", group, key));
    }
}

std::optional<std::string> KeyFile::getStringOpt(
    const std::string& group, const std::string& key) const
{
    if (m_impl->m_keyfile->has_key(group, key)) {
        try {
            return m_impl->m_keyfile->get_string(group, key).raw();
        } catch (Glib::KeyFileError& e) {
            throw std::runtime_error(
                fmt::format("Keyfile Error, no such entry {} {}", group, key));
        }
    }
    return std::nullopt;
}

bool KeyFile::getBoolean(const std::string& group, const std::string& key) const
{
    try {
        return m_impl->m_keyfile->get_boolean(group, key);
    } catch (Glib::KeyFileError& e) {
        throw std::runtime_error(
            fmt::format("Keyfile Error, no such entry {} {}", group, key));
    }
}

std::string KeyFile::toData() const { return m_impl->m_keyfile->to_data(); }

void KeyFile::setString(
    const std::string& group, const std::string& key, const std::string& value)
{
    LOG_ASSERT(group.size() > 0, "Trying to write to file with empty group");
    try {
        m_impl->m_keyfile->set_string(group, key, value);
    } catch (const Glib::Error& e) {
        LOG_ERROR("KeyFile::setString error {}", e.what().raw());
        throw FileException(e.what());
    }
}

void KeyFile::setString(const std::string& group, const std::string& key,
    const std::optional<std::string>& value)
{
    LOG_ASSERT(group.size() > 0, "Trying to write to file with empty group");
    if (value) {
        try {
            m_impl->m_keyfile->set_string(group, key, value.value());
        } catch (const Glib::Error& e) {
            LOG_ERROR("KeyFile::setString error {}", e.what().raw());
            throw FileException(e.what().raw());
        }
    }
}

void KeyFile::setBoolean(
    const std::string& group, const std::string& key, const bool value)
{
    try {
        m_impl->m_keyfile->set_boolean(group, key, value);
    } catch (const Glib::Error& e) {
        LOG_ERROR("KeyFile::setString error {}", e.what().raw());
        throw FileException(e.what());
    }
}

void KeyFile::save() { m_impl->m_keyfile->save_to_file(m_impl->m_path); }

void KeyFile::load() { m_impl->m_keyfile->load_from_file(m_impl->m_path); }

void KeyFile::loadData(const std::string& data) { m_impl->m_keyfile->load_from_data(data); }

std::string checksum(const std::string& data)
{
    Glib::Checksum checksum(Glib::Checksum::ChecksumType::CHECKSUM_SHA256);
    checksum.update(data);
    return checksum.get_string();
}

std::string checksum(
    const std::filesystem::path& path, const std::size_t chunkSize)
{
    Glib::Checksum checksum(Glib::Checksum::ChecksumType::CHECKSUM_SHA256);
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::filesystem::filesystem_error(
            "Failed to open file", path, std::error_code());
    }

    std::vector<std::byte> buffer(chunkSize);

    while (file.read(reinterpret_cast<std::istream::char_type*>(buffer.data()),
        static_cast<std::streamsize>(buffer.size()))) {
        auto bytesRead = static_cast<gsize>(file.gcount());

        checksum.update(
            reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead);
    }

    // Handle any leftover bytes after the while loop ends
    auto bytesRead = static_cast<gsize>(file.gcount());
    if (bytesRead > 0) {
        checksum.update(
            reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead);
    }

    return checksum.get_string();
}

} // namespace cloyster::services::files
