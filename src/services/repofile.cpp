#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/repofile.h>

void ELRepoFile::read()
{
    try {
        m_file = Glib::KeyFile::create();
        m_file->load_from_file(m_path.string());
    } catch (Glib::FileError& e) {
        throw repository_exception(
            std::format("Could not load repository file {} ({})",
                m_path.string(), e.what()));
    }
}

void ELRepoFile::write() { m_file->save_to_file(m_path.string()); }

void ELRepoFile::parse()
{
    read();

    m_repositories = parseData();
}

void ELRepoFile::parse(const std::stringstream& ss)
{
    m_file = Glib::KeyFile::create();
    m_file->load_from_data(ss.str().c_str());
    m_repositories = parseData();
}

std::vector<ELCloneRepo> ELRepoFile::parseData()
{
    auto reponames = m_file->get_groups();

    std::vector<ELCloneRepo> repositories;

    for (const auto& repogroup : reponames) {
        auto name = m_file->get_string(repogroup, "name");

        if (name.empty()) {
            throw repository_exception(std::format(
                "Could not load repo name from repo '{}' at m_file {}",
                repogroup.raw(), m_path.string()));
        }

        auto metalink = cloyster::readKeyfileString(
            m_file, std::string_view { repogroup.c_str() }, "metalink");
        auto baseurl = cloyster::readKeyfileString(
            m_file, std::string_view { repogroup.c_str() }, "baseurl");

        auto enabled = m_file->get_boolean(repogroup, "enabled");
        auto gpgcheck = m_file->get_boolean(repogroup, "gpgcheck");
        auto gpgkey = m_file->get_string(repogroup, "gpgkey");

        ELCloneRepo repo;
        repo.group = repogroup.raw();
        repo.name = name.raw();
        repo.metalink
            = metalink.transform([](const auto& v) { return v.raw(); });
        repo.base_url
            = baseurl.transform([](const auto& v) { return v.raw(); });
        repo.enabled = enabled;
        repo.gpgcheck = gpgcheck;
        repo.gpgkey = gpgkey;
        repositories.push_back(std::move(repo));
    }

    return repositories;
}

void ELRepoFile::unparseData(const std::vector<ELCloneRepo>& repositories)
{
    for (const auto& repo : repositories) {
        m_file->set_string(repo.group, "name", repo.name);
        m_file->set_boolean(repo.group, "enabled", repo.enabled);
        m_file->set_boolean(repo.group, "gpgcheck", repo.gpgcheck);
        m_file->set_string(repo.group, "gpgkey", repo.gpgkey);
    }
}

void ELRepoFile::unparse()
{
    unparseData(m_repositories);
    write();
}

void ELRepoFile::unparse(std::stringstream& ss)
{
    unparseData(m_repositories);
    ss.seekp(0);
    ss << m_file->to_data();
}

[[nodiscard]] std::vector<ELCloneRepo>& ELRepoFile::getRepositories()
{
    return m_repositories;
}

[[nodiscard]] const std::vector<ELCloneRepo>&
ELRepoFile::getRepositoriesConst() const
{
    return m_repositories;
}
