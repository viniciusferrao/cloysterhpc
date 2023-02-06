/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PresenterTimeArea.h"

PresenterTimeArea::PresenterTimeArea(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    // Timezone area selection

    auto availableTimezones = m_model->getTimezone().getAvailableTimezones();

    std::list<std::string> timezoneAreas;
    for (const auto& item : availableTimezones)
        timezoneAreas.emplace_back(item.first);

    timezoneAreas.unique();

    auto selectedTimezoneArea = m_view->listMenu(Messages::title,
        Messages::Timezone::question, timezoneAreas, Messages::Timezone::help);

    m_model->getTimezone().setTimezoneArea(selectedTimezoneArea);

    LOG_DEBUG(
        "Timezone area set to: {}", m_model->getTimezone().getTimezoneArea());
}
