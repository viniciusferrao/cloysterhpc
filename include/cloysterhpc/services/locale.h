/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_LOCALE_H_
#define CLOYSTERHPC_LOCALE_H_

#include <list>
#include <string>

class Locale {
private:
    std::string m_locale;
    std::list<std::string> m_availableLocales;

public:
    Locale();
    void setLocale(std::string_view locale);
    [[nodiscard]] std::string_view getLocale() const;
    [[nodiscard]] std::list<std::string> getAvailableLocales() const;

private:
    [[nodiscard]] std::list<std::string> fetchAvailableLocales() const;
};

#endif // CLOYSTERHPC_LOCALE_H_
