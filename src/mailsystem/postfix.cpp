/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/inifile.h>
#include <cloysterhpc/mailsystem/postfix.h>
#include <cloysterhpc/runner.h>
#include <cloysterhpc/services/log.h>

using cloyster::runCommand;

Postfix::Postfix(
    std::shared_ptr<MessageBus> bus, BaseRunner& runner, Profile profile)
    : IService(bus, "postfix.service")
    , m_runner(runner)
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
    m_runner.executeCommand("dnf -y install postfix");
}

void Postfix::createFiles(const std::filesystem::path& basedir)
{
    LOG_INFO("Creating Postfix configuration files");
    const std::filesystem::path mainFile = basedir / "main.cf";
    const std::filesystem::path masterFile = basedir / "master.cf";
    cloyster::removeFile(masterFile.string());

    inifile baseini;
    baseini.loadData(
#include "cloysterhpc/tmpl/postfix/main.cf.tmpl"
    );

    if (m_hostname) {
        baseini.setValue("", "myhostname", m_hostname.value());
    }

    if (m_domain) {
        baseini.setValue("", "mydomain", m_domain.value());
    }

    if (m_destination) {
        baseini.setValue("", "mydestination",
            fmt::format("{}", fmt::join(m_destination.value(), ",")));
    }

    if (m_cert_file) {
        baseini.setValue("", "smtpd_tls_cert_file", m_cert_file->string());
    }

    if (m_key_file) {
        baseini.setValue("", "smtpd_tls_key_file", m_key_file->string());
    }

    inifile ini = baseini.mergeInto(mainFile);

    //@TODO Check if m_domain is the right key. Maybe a new variable is needed
    // here, m_relayhost_domain?.
    switch (m_profile) {
        case Profile::Local:
            break;
        case Profile::SASL:
        case Profile::Relay:
            if (m_domain && m_port) {
                ini.setValue("", "relayhost",
                    fmt::format("[{}]:{}", m_domain.value(), m_port.value()));
            }
            break;
    }

    ini.saveFile(mainFile);

    const auto& masterConfInclude {
#include "cloysterhpc/tmpl/postfix/master.cf.tmpl"
    };

    std::string masterConf(masterConfInclude);

    if (m_profile != Profile::Local) {
        masterConf
            = cloyster::findAndReplace(masterConf, "{postfix_local_config}",
                "#local     unix  -       n       n       -       -       ");
    } else {
        masterConf
            = cloyster::findAndReplace(masterConf, "{postfix_local_config}",
                "local     unix  -       n       n       -       -       ");
    }

    cloyster::addStringToFile(masterFile.string(), masterConf);

    if (!std::filesystem::exists(basedir / "transport.db")) {
        auto transport = basedir / "transport";
        m_runner.executeCommand(
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
    m_runner.executeCommand(fmt::format("postmap {}", passwordFile.string()));

    std::filesystem::permissions(dbFilename,
        std::filesystem::perms::owner_write
            | std::filesystem::perms::owner_read,
        std::filesystem::perm_options::add);
}

void Postfix::configureRelay(const std::filesystem::path& basedir)
{
    const std::filesystem::path mainFile = basedir / "main.cf";
    inifile ini;
    ini.loadData(mainFile);

    if (m_smtp_server && m_port) {
        ini.setValue("", "relayhost",
            fmt::format("{}:{}", m_smtp_server.value(), m_port.value()));
    }

    ini.saveFile(mainFile);
}

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <cloysterhpc/tempdir.h>
#include <cloysterhpc/tests.h>
#include <testing/test_message_bus.h>

TEST_SUITE("Test repository file read and write")
{
    TEST_CASE("Test if Postfix files generate correctly in the Local profile")
    {
        auto testbus = std::make_shared<TestMessageBus>();
        MockRunner mr;
        Postfix pfix { testbus, mr, Postfix::Profile::Local };
        TempDir d;

        cloyster::touchFile(d.name() / "main.cf");
        cloyster::touchFile(d.name() / "master.cf");

        pfix.setup(d.name());

        CHECK(std::filesystem::file_size(d.name() / "main.cf") > 10);
        CHECK(std::filesystem::file_size(d.name() / "master.cf") > 10);
    }
}
