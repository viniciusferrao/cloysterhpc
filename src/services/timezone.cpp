/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/timezone.h>
#include <fmt/format.h>
#include <map>
#include <string>

Timezone::Timezone()
    : m_availableTimezones { fetchAvailableTimezones() }
{
}

// TODO: Check against m_availableTimezones and throw if not found
void Timezone::setTimezone(std::string_view tz) { m_timezone = tz; }

std::string_view Timezone::getTimezone() const { return m_timezone; }

void Timezone::setTimezoneArea(std::string_view tz) { m_timezoneArea = tz; }

std::string_view Timezone::getTimezoneArea() const { return m_timezoneArea; }

void Timezone::setSystemTimezone()
{
    LOG_DEBUG("Setting system timezone to {}\n", m_timezone)
    cloyster::runCommand(
        fmt::format("timedatectl set timezone {}", m_timezone));
}

std::multimap<std::string, std::string> Timezone::getAvailableTimezones() const
{
    return m_availableTimezones;
}

std::multimap<std::string, std::string> Timezone::fetchAvailableTimezones()
{
    LOG_DEBUG("Fetching available system timezones")
    std::list<std::string> output;

// TODO: Remove this hack
#if __APPLE__
    output.insert(output.end(), { "UTC-3", "GMT", "America/Sao_Paolo" });
#else // Linux or others Unixes
    cloyster::runCommand(
        fmt::format("timedatectl list-timezones --no-pager"), output, true);
#endif

    std::multimap<std::string, std::string> timezones;

    for (const std::string& tz : output) {
        timezones.insert(std::make_pair(
            tz.substr(0, tz.find('/')), tz.substr(tz.find('/') + 1)));
    }

    return timezones;
}

void Timezone::setTimeservers(const std::vector<std::string>& timeservers)
{
    m_timeservers.reserve(timeservers.size());

    for (const auto& timeserver : timeservers)
        m_timeservers.emplace_back(timeserver);
}

// TODO: Check for correctness in timeservers (use hostname/IP check)
// TODO: Remove std::stringstream
// std::stringstream does not support string_view
void Timezone::setTimeservers(const std::string& timeservers)
{
    std::stringstream stream { timeservers };

    while (stream.good()) {
        std::string substring;
        std::getline(stream, substring, ',');

        // Remove spaces from substring
        substring.erase(std::remove(substring.begin(), substring.end(), ' '),
            substring.end());

        m_timeservers.emplace_back(substring);
    }
}

std::vector<std::string> Timezone::getTimeservers() { return m_timeservers; }
