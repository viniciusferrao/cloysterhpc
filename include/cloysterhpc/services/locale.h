/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_LOCALE_H_
#define CLOYSTERHPC_LOCALE_H_

#include <list>
#include <string>

/**
 * @class Locale
 * @brief Manages the locale settings and available locales.
 *
 * This class provides functionalities to set and get the current locale,
 * as well as to retrieve a list of available locales.
 */
class Locale {
private:
    std::string m_locale;
    std::list<std::string> m_availableLocales;

public:
    Locale();
    void setLocale(std::string_view locale);
    [[nodiscard]] std::string_view getLocale() const;

    /**
     * @brief Gets the list of available locales.
     *
     * @return The list of available locales.
     */
    [[nodiscard]] std::list<std::string> getAvailableLocales() const;

private:
    /**
     * @brief Fetches the list of available locales.
     *
     * This method retrieves the list of locales that are available for use.
     *
     * @return The list of available locales.
     */
    [[nodiscard]] std::list<std::string> fetchAvailableLocales() const;
};

#endif // CLOYSTERHPC_LOCALE_H_
