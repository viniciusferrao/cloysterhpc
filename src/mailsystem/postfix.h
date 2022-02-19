//
// Created by Vinícius Ferrão on 19/02/22.
//

#ifndef CLOYSTERHPC_POSTFIX_H
#define CLOYSTERHPC_POSTFIX_H

#include <string>
#include <optional>

class Postfix {
public:
    enum class Profile { Local, Relay, SASL };

private:
    Profile m_profile;
    std::optional<std::string> m_hostname{};
    std::optional<std::uint16_t> m_port{};
    std::optional<std::string> m_username{};
    std::optional<std::string> m_password{};

public:
    [[nodiscard]] const Profile& getProfile() const;
    [[maybe_unused]] void setProfile(Profile profile);

    [[nodiscard]] const std::optional<std::string>& getHostname() const;
    void setHostname(const std::optional<std::string>& hostname);

    [[nodiscard]] const std::optional<std::uint16_t>& getPort() const;
    void setPort(const std::optional<std::uint16_t>& port);

    [[nodiscard]] const std::optional<std::string>& getUsername() const;
    void setUsername(const std::optional<std::string>& username);

    [[nodiscard]] const std::optional<std::string>& getPassword() const;
    void setPassword(const std::optional<std::string>& password);

    explicit Postfix(Profile profile);
};

#endif //CLOYSTERHPC_POSTFIX_H