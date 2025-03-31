/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterQueueSystem.h>

namespace cloyster::presenter {

using cloyster::models::SLURM;

PresenterQueueSystem::PresenterQueueSystem(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    m_model->setQueueSystem(cloyster::utils::enums::ofStringOpt<QueueSystem::Kind>(
        m_view->listMenu(Messages::title, Messages::question,
            cloyster::utils::enums::toStrings<QueueSystem::Kind>(), Messages::help))
            .value());

    // TODO: Placeholder data
    auto fieldsSLURM = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::SLURM::partition, "execution" } });

    if (auto& queue = m_model->getQueueSystem()) {
        switch (queue.value()->getKind()) {
            case QueueSystem::Kind::None: {
                __builtin_unreachable();
            }

            case QueueSystem::Kind::SLURM: {
                fieldsSLURM = m_view->fieldMenu(Messages::SLURM::title,
                    Messages::SLURM::question, fieldsSLURM,
                    Messages::SLURM::help);

                const auto& slurm = dynamic_cast<SLURM*>(queue.value().get());
                slurm->setDefaultQueue(fieldsSLURM[0].second);
                LOG_DEBUG(
                    "Set SLURM default queue: {}", slurm->getDefaultQueue());

                break;
            }

            case QueueSystem::Kind::PBS: {
                const auto& execution = m_view->listMenu(Messages::PBS::title,
                    Messages::PBS::question,
                    cloyster::utils::enums::toStrings<PBS::ExecutionPlace>(),
                    Messages::PBS::help);

                const auto& pbs = dynamic_cast<PBS*>(queue.value().get());
                pbs->setExecutionPlace(
                    cloyster::utils::enums::ofStringOpt<PBS::ExecutionPlace>(execution)
                        .value());
                LOG_DEBUG("Set PBS Execution Place: {}",
                    cloyster::utils::enums::toString<PBS::ExecutionPlace>(
                        pbs->getExecutionPlace()));

                break;
            }
        }
    }
}
}
