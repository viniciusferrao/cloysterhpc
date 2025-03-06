#include <concepts>
#include <ranges>
#include <stdexcept>
#include <utility>

#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>

namespace cloyster::services::files {

struct KeyFile::Impl {
    std::filesystem::path m_path;
    std::unique_ptr<Glib::KeyFile> m_keyfile;

    Impl(Glib::KeyFile&& keyfile, std::filesystem::path path)
        : m_path(std::move(path))
        , m_keyfile(std::make_unique<Glib::KeyFile>(std::move(keyfile))) { };

    void safeToFile(const std::filesystem::path& path)
    {
        try {
            m_keyfile->save_to_file(path);
        } catch (std::runtime_error& e) {
            LOG_ERROR(
                "Error while loading key file {}: {}", path.string(), e.what());
            throw FileException(e.what());
        }
    }

    void loadFromFile(const std::filesystem::path& path)
    {
        try {
            m_keyfile->load_from_file(path);
        } catch (std::runtime_error& e) {
            LOG_ERROR(
                "Error while loading key file {}: {}", path.string(), e.what());
            throw FileException(e.what());
        }
    }

    void loadFromData(const std::string& data)
    {
        try {
            m_keyfile->load_from_data(data);
        } catch (std::runtime_error& e) {
            LOG_ERROR(
                "Error while loading key from data '{}': {}", data, e.what());
            throw FileException(e.what());
        }
    }
};

KeyFile::KeyFile(KeyFile::Impl&& impl)
    : m_impl(std::make_unique<KeyFile::Impl>(std::move(impl)))
{
}

KeyFile::KeyFile(const std::filesystem::path& path)
    : m_impl(std::make_unique<KeyFile::Impl>(Glib::KeyFile(), path))
{
    m_impl->m_path = path;
    m_impl->loadFromFile(path);
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

std::vector<std::string> KeyFile::getGroups() const
{
    return toStrings(m_impl->m_keyfile->get_groups());
}

std::string KeyFile::getString(
    const std::string& group, const std::string& key) const
{
    return m_impl->m_keyfile->get_string(group, key).raw();
}

std::optional<std::string> KeyFile::getStringOpt(
    const std::string& group, const std::string& key) const
{
    if (m_impl->m_keyfile->has_key(group, key)) {
        return m_impl->m_keyfile->get_string(group, key).raw();
    }
    return std::nullopt;
}

bool KeyFile::getBoolean(const std::string& group, const std::string& key) const
{
    return m_impl->m_keyfile->get_boolean(group, key);
}

std::string KeyFile::toData() const { return m_impl->m_keyfile->to_data(); }

void KeyFile::setString(
    const std::string& group, const std::string& key, const std::string& value)
{
    m_impl->m_keyfile->set_string(group, key, value);
}

void KeyFile::setString(const std::string& group, const std::string& key,
    const std::optional<std::string>& value)
{
    if (value) {
        m_impl->m_keyfile->set_string(group, key, value.value());
    }
}

void KeyFile::setBoolean(
    const std::string& group, const std::string& key, const bool value)
{
    m_impl->m_keyfile->set_boolean(group, key, value);
}

void KeyFile::save() { m_impl->safeToFile(m_impl->m_path); }

void KeyFile::load() { m_impl->loadFromFile(m_impl->m_path); }

} // namespace cloyster::services::files
