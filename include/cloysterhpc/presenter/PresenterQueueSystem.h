/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_
#define CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterQueueSystem : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Queue System settings";
        static constexpr const auto question
            = "Pick a queue system to run you compute jobs";
        static constexpr const auto help
            = Presenter::Messages::Placeholder::help;

        struct SLURM {
            static constexpr const auto title = "SLURM settings";
            static constexpr const auto question
                = "Enter the default partition name for SLURM";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto partition = "Partition name";
        };

        struct PBS {
            static constexpr const auto title = "PBS Professional settings";
            static constexpr const auto question
                = "Select the default execution place for PBS Professional "
                  "jobs";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterQueueSystem(
        std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_
