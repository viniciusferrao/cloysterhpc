/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERINFINIBAND_H_
#define CLOYSTERHPC_PRESENTERINFINIBAND_H_

#include <cloysterhpc/presenter/Presenter.h>
#include <cloysterhpc/presenter/PresenterNetwork.h>

class PresenterInfiniband : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Infiniband settings";
        static constexpr const char* question
            = "Do you have an Infiniband Fabric available?";
        static constexpr const char* help
            = Presenter::Messages::Placeholder::help;

        struct OFED {
            static constexpr const char* question
                = "Choose the desired Infiniband stack";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterInfiniband(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERINFINIBAND_H_
