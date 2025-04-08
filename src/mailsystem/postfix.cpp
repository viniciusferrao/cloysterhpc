/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <iostream>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/mailsystem/postfix.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/runner.h>

namespace cloyster::services {

using cloyster::services::IRunner;
using cloyster::services::files::KeyFile;

Postfix::Postfix(Profile profile)
    : IService("postfix.service")
    , m_profile(profile)
{
}

const Postfix::Profile& Postfix::getProfile() const { return m_profile; }

void Postfix::setProfile(Profile profile) { m_profile = profile; }

const std::optional<std::string>& Postfix::getHostname() const
{
    return m_hostname;
}
void Postfix::setHostname(const std::optional<std::string>& hostname)
{
    m_hostname = hostname;
}

const std::optional<std::string>& Postfix::getDomain() const
{
    return m_domain;
}

void Postfix::setDomain(const std::optional<std::string>& domain)
{
    m_domain = domain;
}

const std::optional<std::uint16_t>& Postfix::getPort() const { return m_port; }

void Postfix::setPort(const std::optional<std::uint16_t>& port)
{
    m_port = port;
}

const std::optional<std::string>& Postfix::getUsername() const
{
    return m_username;
}

void Postfix::setUsername(const std::optional<std::string>& username)
{
    m_username = username;
}

const std::optional<std::string>& Postfix::getPassword() const
{
    return m_password;
}

void Postfix::setPassword(const std::optional<std::string>& password)
{
    m_password = password;
}

const std::optional<std::vector<std::string>>& Postfix::getDestination() const
{
    return m_destination;
}

void Postfix::setDestination(
    const std::optional<std::vector<std::string>>& destination)
{
    m_destination = destination;
}

const std::optional<std::string>& Postfix::getSMTPServer() const
{
    return m_smtp_server;
}

void Postfix::setSMTPServer(const std::optional<std::string>& smtp_server)
{
    m_smtp_server = smtp_server;
}

const std::optional<std::filesystem::path>& Postfix::getCertFile() const
{
    return m_cert_file;
}

void Postfix::setCertFile(const std::optional<std::filesystem::path>& cert_file)
{
    m_cert_file = cert_file;
}

const std::optional<std::filesystem::path>& Postfix::getKeyFile() const
{
    return m_key_file;
}

void Postfix::setKeyFile(const std::optional<std::filesystem::path>& key_file)
{
    m_key_file = key_file;
}

void Postfix::install()
{
    LOG_INFO("Installing Postfix");
    cloyster::Singleton<IRunner>::get()->executeCommand(
        "dnf -y install postfix");
}

static void maybeDisableLocalOnMasterFile(std::string& line)
{
    std::vector<std::string> linedata;

    boost::split(
        linedata, line, boost::is_any_of("\t "), boost::token_compress_on);
    if (linedata[0] == "local") {
        std::string out = "#" + line;
        line = out;
    }
}

void Postfix::changeMasterFile(const std::filesystem::path& masterFile)
{
    std::vector<std::string> lines;
    std::ifstream iread(masterFile);

    while (iread.good()) {
        std::string line;
        std::getline(iread, line);

        if (line[0] != '#' && m_profile != Profile::Local) {
            maybeDisableLocalOnMasterFile(line);
        }

        lines.push_back(line);
    }
    iread.close();

    std::ofstream iwrite(masterFile);
    for (const auto& l : lines) {
        iwrite << l;
    }

    iwrite.close();
}

void Postfix::createFiles(const std::filesystem::path& basedir)
{
    LOG_INFO("Creating Postfix configuration files");
    const std::filesystem::path mainFile = basedir / "main.cf";
    const std::filesystem::path masterFile = basedir / "master.cf";

    KeyFile baseini(mainFile);
    baseini.loadData(
#include "cloysterhpc/tmpl/postfix/main.cf.tmpl"
    );

    if (m_hostname) {
        baseini.setString("", "myhostname", m_hostname.value());
    }

    if (m_domain) {
        baseini.setString("", "mydomain", m_domain.value());
    }

    if (m_destination) {
        baseini.setString("", "mydestination",
            fmt::format("{}", fmt::join(m_destination.value(), ",")));
    }

    if (m_cert_file && m_key_file) {
        baseini.setString("", "smtpd_tls_cert_file", m_cert_file->string());
        baseini.setString("", "smtpd_tls_key_file", m_key_file->string());
    }

    // @FIXME: KeyFile does not have mergeInto, I didn't undestand why this
    //   is necessary (it seems it is not) so I commented for now, but the
    //   below code may be broken.
    //   Test postfix file generation and remove this comment
    //
    // KeyFile ini = baseini.mergeInto(mainFile);

    //@TODO Check if m_domain is the right key. Maybe a new variable is needed
    // here, m_relayhost_domain?.
    switch (m_profile) {
        case Profile::Local:
            break;
        case Profile::SASL:
        case Profile::Relay:
            if (m_domain && m_port) {
                baseini.setString("", "relayhost",
                    fmt::format("[{}]:{}", m_domain.value(), m_port.value()));
            }
            break;
    }

    baseini.save();

    changeMasterFile(masterFile);

    if (!std::filesystem::exists(basedir / "transport.db")) {
        auto transport = basedir / "transport";
        cloyster::Singleton<IRunner>::get()->executeCommand(
            fmt::format("postmap hash:{}", transport.string()));
    }

    if (!std::filesystem::exists(basedir / "aliases")) {
        cloyster::touchFile(basedir / "aliases");
    }
}

void Postfix::setup(const std::filesystem::path& basedir)
{
    LOG_INFO("Configuring Postfix")

    install();
    createFiles(basedir);

    switch (m_profile) {

        case Profile::Local:
            break;
        case Profile::Relay:
            configureRelay(basedir);
            break;
        case Profile::SASL:
            configureSASL(basedir);
            break;
    }

    enable();
    start();
}

void Postfix::configureSASL(const std::filesystem::path& basedir)
{
    const std::filesystem::path dbFilename = basedir / "sasl_password.db";
    const std::filesystem::path filename = basedir / "sasl_password";
    if (std::filesystem::exists(dbFilename)) {
        return;
    }

    cloyster::addStringToFile(filename.string(),
        fmt::format("[{}]:{} {}:{}", m_smtp_server.value(), m_port.value(),
            m_username.value(), m_password.value()));

    std::filesystem::permissions(filename,
        std::filesystem::perms::owner_write | std::filesystem::perms::owner_read
            | std::filesystem::perms::group_read,
        std::filesystem::perm_options::add);

    auto passwordFile = basedir / "sasl_password";
    cloyster::Singleton<IRunner>::get()->executeCommand(
        fmt::format("postmap {}", passwordFile.string()));

    std::filesystem::permissions(dbFilename,
        std::filesystem::perms::owner_write
            | std::filesystem::perms::owner_read,
        std::filesystem::perm_options::add);
}

void Postfix::configureRelay(const std::filesystem::path& basedir)
{
    const std::filesystem::path mainFile = basedir / "main.cf";
    KeyFile ini(mainFile);
    ;

    if (m_smtp_server && m_port) {
        ini.setString("", "relayhost",
            fmt::format("{}:{}", m_smtp_server.value(), m_port.value()));
    }

    ini.save();
}

}; // namespace cloyster::services
