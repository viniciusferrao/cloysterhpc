/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_
#define CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_

#include "Presenter.h"

class PresenterQueueSystem : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Queue System settings";
        static constexpr const char* question
            = "Pick a queue system to run you compute jobs";
        static constexpr const char* help
            = Presenter::Messages::Placeholder::help;

        struct SLURM {
            static constexpr const char* title = "SLURM settings";
            static constexpr const char* question
                = "Enter the default partition name for SLURM";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;

            static constexpr const char* partition = "Partition name";
        };

        struct PBS {
            static constexpr const char* title = "PBS Professional settings";
            static constexpr const char* question
                = "Select the default execution place for PBS Professional "
                  "jobs";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterQueueSystem(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERQUEUESYSTEM_H_
