//
// Created by Vinícius Ferrão on 11/01/22.
//

#include "log.h"
#include "timezone.h"
#include "../functions.h"
#include <fmt/format.h>

void Timezone::setTimezone(std::string_view tz)
{
    m_timezone = tz;
}

std::string_view Timezone::getTimezone()
{
    return m_timezone;
}

void Timezone::setSystemTimezone()
{
    LOG_DEBUG("Setting system timezone to {}\n", m_timezone);
    cloyster::runCommand(
            fmt::format("timedatectl set timezone {}", m_timezone));
}

std::vector<std::string> Timezone::getAvailableTimezones()
{
    LOG_TRACE("Fetching available system timezones\n");
    std::vector<std::string> output;
    cloyster::runCommand(fmt::format("timedatectl list-timezones"), output);
    return output;
}
