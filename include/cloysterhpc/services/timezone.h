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

/**
 * @class Timezone
 * @brief Manages the system timezone and time servers.
 *
 * This class provides functionalities to set and get the system timezone,
 * manage available timezones, and configure time servers.
 */
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

    /**
     * @brief Sets the current timezone.
     *
     * @param timezone The timezone to set, as a string view.
     */
    void setTimezone(std::string_view);

    /**
     * @brief Gets the current timezone.
     *
     * @return The current timezone as a string view.
     */
    std::string_view getTimezone() const;

    /**
     * @brief Sets the system timezone.
     *
     * This function configures the system's timezone based on the current
     * settings.
     */
    void setSystemTimezone();

    /**
     * @brief Gets the list of available timezones.
     *
     * @return A multimap of available timezones.
     */
    std::multimap<std::string, std::string> getAvailableTimezones() const;

    /**
     * @brief Fetches the list of available timezones.
     *
     * This function retrieves the available timezones from the system or a
     * database.
     *
     * @return A multimap of available timezones.
     */
    std::multimap<std::string, std::string> fetchAvailableTimezones();

    void setTimezoneArea(std::string_view);
    std::string_view getTimezoneArea() const;

    void setTimeservers(const std::vector<std::string>& timeservers);
    void setTimeservers(const std::string& timeservers);
    std::vector<std::string> getTimeservers();
};

#endif // CLOYSTERHPC_TIMEZONE_H_
