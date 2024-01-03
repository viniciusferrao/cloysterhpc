/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/services/locale.h"
#include "cloysterhpc/functions.h"
#include <cloysterhpc/services/log.h>

Locale::Locale()
    : m_availableLocales { fetchAvailableLocales() }
{
}

void Locale::setLocale(std::string_view locale)
{
    if (std::find(m_availableLocales.begin(), m_availableLocales.end(), locale)
        != m_availableLocales.end())
        m_locale = locale;
    else
        throw std::runtime_error("Unsupported locale");
}

std::string_view Locale::getLocale() const { return m_locale; }

std::list<std::string> Locale::getAvailableLocales() const
{
    return m_availableLocales;
}

std::list<std::string> Locale::fetchAvailableLocales() const
{
    LOG_DEBUG("Fetching available system locales")
    std::list<std::string> output;

    cloyster::runCommand(fmt::format("locale -a"), output, true);

    return output;
}
