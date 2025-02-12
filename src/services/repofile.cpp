#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/repofile.h>
#include <stdexcept>


namespace cloyster {

namespace {

using cloyster::services::files::IsKeyFileReadable;
template <IsKeyFileReadable KeyFile, IsRepository Repository = RPMRepository>
void parseData(KeyFile& file, std::vector<Repository> repositories)
{
    auto reponames = file.getGroups();
    repositories.reserve(reponames.size());

    for (const auto& repogroup : reponames) {
        auto name = file.getString(repogroup, "name");

        if (name.empty()) {
            throw std::runtime_error(std::format(
                "Could not load repo name from repo '{}'",
                repogroup));
        }

        auto metalink = file.getStringOpt(repogroup, "metalink");
        auto baseurl = file.getStringOpt(repogroup, "baseurl");
        auto enabled = file.getBoolean(repogroup, "enabled");
        auto gpgcheck = file.getBoolean(repogroup, "gpgcheck");
        auto gpgkey = file.getString(repogroup, "gpgkey");

        RPMRepository repo;
        repo.group = repogroup;
        repo.name = name;
        repo.metalink = metalink;
        repo.baseurl = baseurl;
        repo.enabled = enabled;
        repo.gpgcheck = gpgcheck;
        repo.gpgkey = gpgkey;
        repositories.emplace_back(std::move(repo));
    }
}

using cloyster::services::files::IsKeyFileWriteable;
template <IsKeyFileWriteable KeyFile>
void unparseData(const std::vector<RPMRepository>& repositories, KeyFile& file)
{
    for (const auto& repo : repositories) {
        file.setString(repo.group, "name", repo.name);
        file.setBoolean(repo.group, "enabled", repo.enabled);
        file.setBoolean(repo.group, "gpgcheck", repo.gpgcheck);
        file.setString(repo.group, "gpgkey", repo.gpgkey);
        // uhh spooky, setString over optional only write if the
        // optional has a value
        file.setString(repo.group, "metalink", repo.metalink);
        file.setString(repo.group, "baseurl", repo.metalink);
    }
}
}; 


void RPMRepositoryParser::parse(std::istream& input, std::vector<RPMRepository>& output) {
    auto keyfile = cloyster::services::files::KeyFile(input);
    parseData(keyfile, output);
}

void RPMRepositoryParser::unparse(const std::vector<RPMRepository>& repos, std::ostream& output) {
    std::istringstream input("");
    auto keyfile = cloyster::services::files::KeyFile(input);
    unparseData(repos, keyfile);
}
/*
void ELRepoFile::read()
{
    try {
        m_file = Glib::KeyFile::create();
        m_file->load_from_file(m_path.string());
    } catch (Glib::FileError& e) {
        throw RepositoryException(
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
*/

} // namespace cloyster
