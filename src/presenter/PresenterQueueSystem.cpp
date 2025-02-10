/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterQueueSystem.h>

PresenterQueueSystem::PresenterQueueSystem(
    std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    m_model->setQueueSystem(magic_enum::enum_cast<QueueSystem<BaseRunner>::Kind>(
        m_view->listMenu(Messages::title, Messages::question,
            magic_enum::enum_names<QueueSystem<BaseRunner>::Kind>(), Messages::help))
            .value());

    // TODO: Placeholder data
    auto fieldsSLURM = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::SLURM::partition, "execution" } });

    if (auto& queue = m_model->getQueueSystem()) {
        using QueueSystem = QueueSystem<BaseRunner>;
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
                    magic_enum::enum_names<PBS::ExecutionPlace>(),
                    Messages::PBS::help);

                const auto& pbs = dynamic_cast<PBS*>(queue.value().get());
                pbs->setExecutionPlace(
                    magic_enum::enum_cast<PBS::ExecutionPlace>(execution)
                        .value());
                LOG_DEBUG("Set PBS Execution Place: {}",
                    magic_enum::enum_name<PBS::ExecutionPlace>(
                        pbs->getExecutionPlace()));

                break;
            }
        }
    }
}
