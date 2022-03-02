//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterTime.h"

PresenterTime::PresenterTime(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    // Timezone selection
    auto availableTimezones = m_model->getTimezone()
                                      .getAvailableTimezones();

    auto selectedTimezone = m_view->listMenu(
            Messages::title,
            Messages::Timezone::question,
            availableTimezones,
            Messages::Timezone::help);

    m_model->setTimezone(selectedTimezone);

    // FIXME: Horrible call; getTimezone() two times? Srsly?
    LOG_TRACE("Timezone set to: {}", m_model->getTimezone().getTimezone());

    // Timeserver settings
    // FIXME: Fix the interface hack to show only one time "time server"
    auto timeservers = std::to_array<
            std::pair<std::string, std::string>>({
                 {Messages::Timeservers::field, "0.br.pool.ntp.org"},
                 {"", "1.br.pool.ntp.org"},
                 {"", "2.br.pool.ntp.org"}
            });

    // TODO: Set timeservers
    timeservers = m_view->fieldMenu(Messages::title,
                                    Messages::Timeservers::question,
                                    timeservers,
                                    Messages::Timeservers::help);

//  m_model->getTimezone().setTimeservers(timeservers);
//  LOG_INFO("Timeservers set to {}", m_model->getTimezone().getTimeservers());
}
