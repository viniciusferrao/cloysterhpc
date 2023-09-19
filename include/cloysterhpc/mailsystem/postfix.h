/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_POSTFIX_H_
#define CLOYSTERHPC_POSTFIX_H_

#include <optional>
#include <string>
#include <cstdint>

class Postfix {
public:
    enum class Profile { Local, Relay, SASL };

private:
    Profile m_profile;
    std::optional<std::string> m_hostname {};
    std::optional<std::string> m_domain {};
    std::optional<std::string> m_fqdn {};
    std::optional<std::string> m_smtp_server {};
    std::optional<std::vector<std::string>> m_destination {};
    std::optional<std::uint16_t> m_port {};
    std::optional<std::string> m_username {};
    std::optional<std::string> m_password {};
    void install();
    void createFiles();
    void configureSASL();
    void configureRelay();

public:
    [[nodiscard]] const Profile& getProfile() const;
    [[maybe_unused]] void setProfile(Profile profile);
    [[nodiscard]] const std::optional<std::string>& getHostname() const;

    void setHostname(const std::optional<std::string>& hostname);

    const std::optional<std::vector<std::string>>& getDestination() const;
    void setDestination(
        const std::optional<std::vector<std::string>>& destination);

    [[nodiscard]] const std::optional<std::string>& getDomain() const;
    void setDomain(const std::optional<std::string>& domain);

    [[nodiscard]] const std::optional<std::uint16_t>& getPort() const;
    void setPort(const std::optional<std::uint16_t>& port);

    [[nodiscard]] const std::optional<std::string>& getUsername() const;
    void setUsername(const std::optional<std::string>& username);

    [[nodiscard]] const std::optional<std::string>& getPassword() const;
    void setPassword(const std::optional<std::string>& password);

    [[nodiscard]] const std::optional<std::string>& getFQDN() const;
    void setFQDN(const std::optional<std::string>& fqdn);

    [[nodiscard]] const std::optional<std::string>& getSMTPServer() const;
    void setSMTPServer(const std::optional<std::string>& smtp_server);

    void setup();
    void enable();
    void disable();
    void start();
    void restart();
    void stop();

    explicit Postfix(Profile profile);
};

#endif // CLOYSTERHPC_POSTFIX_H_
