/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PresenterTime.h"

PresenterTime::PresenterTime(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    // Timezone area selection

    auto availableTimezones = m_model->getTimezone().getAvailableTimezones();

    std::list<std::string> timezoneAreas;
    for (const auto& tz : availableTimezones)
        timezoneAreas.emplace_back(tz.first);

    timezoneAreas.unique();

    auto selectedTimezoneLocationArea = m_view->listMenu(Messages::title,
        Messages::Timezone::question, timezoneAreas, Messages::Timezone::help);

    m_model->getTimezone().setTimezoneArea(selectedTimezoneLocationArea);

    std::string_view timezoneArea = m_model->getTimezone().getTimezoneArea();

    LOG_DEBUG(
        "Timezone area set to: {}", timezoneArea);

    // Timezone location selection

    std::list<std::string> timezoneLocations;
    for (const auto& item : availableTimezones)
        if (item.first == timezoneArea)
            timezoneLocations.emplace_back(item.second);

    auto selectedTimezoneLocation = m_view->listMenu(Messages::title,
        Messages::Timezone::question, timezoneLocations, Messages::Timezone::help);

    m_model->setTimezone(fmt::format("{}/{}", timezoneArea, selectedTimezoneLocation));

    // FIXME: Horrible call; getTimezone() two times? Srsly?
    LOG_DEBUG("Timezone set to: {}", m_model->getTimezone().getTimezone());

    // Timeserver settings
    // FIXME: We left std::to_array if we want to manage the input on the view,
    //        making setTimeservers a little clunky.
    auto timeservers = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::Timeservers::field, "0.br.pool.ntp.org" } });

    timeservers
        = m_view->fieldMenu(Messages::title, Messages::Timeservers::question,
            timeservers, Messages::Timeservers::help);

    m_model->getTimezone().setTimeservers(timeservers[0].second);
    LOG_DEBUG("Timeservers set to {}",
        fmt::join(m_model->getTimezone().getTimeservers(), ", "));
}
