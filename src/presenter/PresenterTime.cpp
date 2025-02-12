/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterTime.h>
#include <set>

namespace cloyster::presenter {

PresenterTime::PresenterTime(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    // Timezone area selection

    auto availableTimezones = m_model->getTimezone().getAvailableTimezones();

    std::set<std::string> timezoneAreas;
    for (const auto& tz : availableTimezones)
        timezoneAreas.insert(tz.first);

    auto selectedTimezoneLocationArea = m_view->listMenu(Messages::title,
        Messages::Timezone::question, timezoneAreas, Messages::Timezone::help);

    m_model->getTimezone().setTimezoneArea(selectedTimezoneLocationArea);

    std::string_view timezoneArea = m_model->getTimezone().getTimezoneArea();

    LOG_DEBUG("Timezone area set to: {}", timezoneArea)

    // Timezone location selection

    std::list<std::string> timezoneLocations;
    const auto& [begin, end]
        = availableTimezones.equal_range(timezoneArea.data());
    for (auto it = begin; it != end; ++it) {
        timezoneLocations.emplace_back(it->second);
    }

    auto selectedTimezoneLocation
        = m_view->listMenu(Messages::title, Messages::Timezone::question,
            timezoneLocations, Messages::Timezone::help);

    m_model->setTimezone(
        fmt::format("{}/{}", timezoneArea, selectedTimezoneLocation));

    // FIXME: Horrible call; getTimezone() two times? Srsly?
    LOG_DEBUG("Timezone set to: {}", m_model->getTimezone().getTimezone())

    std::vector<std::string> defaultServers = { "0.br.pool.ntp.org" };

    auto collectCallback = [this](std::vector<std::string>& items) {
        // Timeserver settings
        // FIXME: We left std::to_array if we want to manage the input on the
        // view,
        //        making setTimeservers a little clunky.
        auto timeserver = std::to_array<std::pair<std::string, std::string>>(
            { { Messages::AddTimeserver::field, "" } });

        timeserver = m_view->fieldMenu(Messages::title,
            Messages::AddTimeserver::question, timeserver,
            Messages::AddTimeserver::help);

        items.push_back(timeserver[0].second);

        return true;
    };

    auto timeservers = m_view->collectListMenu(Messages::title,
        Messages::Timeservers::question, defaultServers,
        Messages::Timeservers::help, std::move(collectCallback));

    m_model->getTimezone().setTimeservers(timeservers);
    LOG_DEBUG("Timeservers set to {}",
        fmt::join(m_model->getTimezone().getTimeservers(), ", "));
}

};
