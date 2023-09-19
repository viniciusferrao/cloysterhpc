/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/inifile.h"
#include <cloysterhpc/functions.h>
#include <cloysterhpc/mailsystem/postfix.h>
#include <cloysterhpc/services/log.h>

using cloyster::runCommand;

Postfix::Postfix(Profile profile)
    : m_profile(profile)
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

const std::optional<std::string>& Postfix::getFQDN() const { return m_fqdn; }

void Postfix::setFQDN(const std::optional<std::string>& fqdn) { m_fqdn = fqdn; }

const std::optional<std::string>& Postfix::getSMTPServer() const { return m_smtp_server; }

void Postfix::setSMTPServer(const std::optional<std::string>& smtp_server) { m_smtp_server = smtp_server; }

void Postfix::install()
{
    LOG_INFO("Installing Postfix")
    runCommand("dnf -y install postfix");
}

void Postfix::createFiles()
{
    LOG_INFO("Creating Postfix configuration files");
    const std::string mainFile { "/etc/postfix/main.cf" };
    cloyster::removeFile(mainFile);
    const std::string masterFile { "/etc/postfix/master.cf" };
    cloyster::removeFile(masterFile);

    inifile ini;
    ini.loadData(
#include "cloysterhpc/tmpl/postfix/main.cf.tmpl"
    );

    ini.setValue("", "myhostname", m_hostname.value());
    ini.setValue("", "mydomain", m_domain.value());
    ini.setValue("", "mydestination",
        fmt::format("{}", fmt::join(m_destination.value(), ",")));
    ini.setValue("", "smtpd_tls_cert_file",
        fmt::format("/etc/pki/tls/certs/{}.cer", m_fqdn.value()));
    ini.setValue("", "smtpd_tls_key_file",
        fmt::format("/etc/pki/tls/private/{}.key", m_fqdn.value()));

    //@TODO Check if m_domain is the right key. Maybe a new variable is needed
    //here, m_relayhost_domain?.
    switch (m_profile) {
        case Profile::Local:
            break;
        case Profile::SASL:
            ini.setValue("", "relayhost",
                fmt::format("{}:{}", m_domain.value(), m_port.value()));
            break;
        case Profile::Relay:
            ini.setValue("", "relayhost",
                fmt::format("[{}]:{}", m_domain.value(), m_port.value()));
            break;
    }

    ini.saveFile(mainFile);

    const auto& masterConf {
#include "cloysterhpc/tmpl/postfix/master.cf.tmpl"
    };

    cloyster::addStringToFile(masterFile, masterConf);

    //@TODO Not the best way to do this, but the only one who worked now. Gotta
    //fix it.
    if (m_profile != Profile::Local) {
        cloyster::runCommand(
            "sed -i 's/local     unix  -       n       n       -       -       "
            "local/#local     unix  -       n       n       -       -       "
            "local/g' /etc/postfix/master.cf");
    }

    if (!std::filesystem::exists("/etc/postfix/transport.db")) {
        runCommand("postmap hash:/etc/postfix/transport");
    }

    if (!std::filesystem::exists("/etc/postfix/aliases")) {
        runCommand("touch /etc/postfix/aliases");
    }
}

void Postfix::setup()
{
    LOG_INFO("Configuring Postfix")

    install();
    createFiles();

    switch (m_profile) {

        case Profile::Local:
            break;
        case Profile::Relay:
            configureRelay();
            break;
        case Profile::SASL:
            configureSASL();
            break;
    }

    enable();
    start();
}

void Postfix::configureSASL()
{
    const std::string dbFilename { "/etc/postfix/sasl_password.db" };
    const std::string filename { "/etc/postfix/sasl_password" };
    if (std::filesystem::exists(dbFilename)) {
        return;
    }

    cloyster::addStringToFile(filename,
        fmt::format("[{}]:{} {}:{}", m_smtp_server.value(), m_port.value(),
            m_username.value(), m_password.value()));

    std::filesystem::permissions(filename,
        std::filesystem::perms::owner_write | std::filesystem::perms::owner_read
            | std::filesystem::perms::group_read,
        std::filesystem::perm_options::add);

    runCommand("postmap /etc/postfix/sasl_password");

    std::filesystem::permissions(dbFilename,
        std::filesystem::perms::owner_write
            | std::filesystem::perms::owner_read,
        std::filesystem::perm_options::add);
}

//@TODO
void Postfix::configureRelay() { }

void Postfix::enable() { runCommand("systemctl enable postfix"); }

void Postfix::disable() { runCommand("systemctl disable postfix"); }
void Postfix::start() { runCommand("systemctl start postfix"); }
void Postfix::restart() { runCommand("systemctl restart postfix"); }
void Postfix::stop() { runCommand("systemctl stop postfix"); }
