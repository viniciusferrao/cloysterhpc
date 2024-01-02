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
    ~Locale() = default;

    void setLocale(std::string_view locale);
    std::string_view getLocale() const;

    std::list<std::string> getAvailableLocales() const;
    std::list<std::string> fetchAvailableLocales() const;
};

#endif // CLOYSTERHPC_LOCALE_H_
