/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_TIMEZONE_H_
#define CLOYSTERHPC_TIMEZONE_H_

#include <list>
#include <map>
#include <string>
#include <vector>

class Timezone {
private:
    std::string m_timezone;
    std::string m_timezoneArea;
    std::multimap<std::string, std::string> m_availableTimezones;
    std::multimap<std::string, std::string> m_availableTimezoneAreas;
    // TODO: IP/hostname parsing
    std::vector<std::string> m_timeservers;

public:
    Timezone();
    ~Timezone() = default;

    void setTimezone(std::string_view);
    std::string_view getTimezone() const;

    void setSystemTimezone();
    std::multimap<std::string, std::string> getAvailableTimezones() const;
    std::multimap<std::string, std::string> fetchAvailableTimezones();

    void setTimezoneArea(std::string_view);
    std::string_view getTimezoneArea() const;

    void setTimeservers(const std::vector<std::string>& timeservers);
    void setTimeservers(const std::string& timeservers);
    std::vector<std::string> getTimeservers();
};

#endif // CLOYSTERHPC_TIMEZONE_H_
