#include <ranges>
#include <utility>

#include <glibmm/fileutils.h>
#include <glibmm/keyfile.h>

#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>

namespace cloyster::services::files {

struct KeyFile::Impl {
    std::filesystem::path m_path;
    Glib::RefPtr<Glib::KeyFile> m_keyfile;

    Impl(Glib::RefPtr<Glib::KeyFile>&& keyfile, std::filesystem::path path)
        : m_path(std::move(path))
        , m_keyfile(std::move(keyfile))
    {
    };
};

KeyFile::KeyFile(KeyFile::Impl&& impl)
    : m_impl(std::make_unique<KeyFile::Impl>(std::move(impl)))
{
}

KeyFile::KeyFile(const std::filesystem::path& path)
{
    m_impl = std::make_unique<KeyFile::Impl>(Glib::KeyFile::create(), path);
    m_impl->m_keyfile->load_from_file(path);
    m_impl->m_path = path;
}

KeyFile::KeyFile(std::istream& istream)
{
    m_impl = std::make_unique<KeyFile::Impl>(Glib::KeyFile::create(), std::filesystem::path());
    std::istreambuf_iterator<char> begin(istream);
    std::istreambuf_iterator<char> end;
    std::string data(begin, end);
    m_impl->m_keyfile->load_from_data(data);
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
    auto keyFile = m_impl->m_keyfile;
    if (keyFile->has_key(group, key)) {
        return keyFile->get_string(group, key).raw();
    }
    return std::nullopt;
}

bool KeyFile::getBoolean(const std::string& group, const std::string& key) const
{
    return m_impl->m_keyfile->get_boolean(group, key);
}

void KeyFile::setString(const std::string& group, const std::string& key, const std::string& value) {
    m_impl->m_keyfile->set_string(group, key, value);
}

void KeyFile::setString(const std::string& group, const std::string& key, const std::optional<std::string>& value) {
    if (value) {
        m_impl->m_keyfile->set_string(group, key, value.value());
    }
}

void KeyFile::setBoolean(const std::string& group, const std::string& key, const bool value) {
    m_impl->m_keyfile->set_boolean(group, key, value);
}

void KeyFile::save()
{
    m_impl->m_keyfile->save_to_file(m_impl->m_path);
}

void KeyFile::load()
{
    m_impl->m_keyfile->load_from_file(m_impl->m_path);
}

} // namespace cloyster::services::files
