/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/mailsystem/postfix.h>

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
