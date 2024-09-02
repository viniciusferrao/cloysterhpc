/*
 * Copyright 2024 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cloysterhpc/presenter/Presenter.h>

class PresenterRepository : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Repository Selection";

        struct General {
            static constexpr const auto question
                = "Enable/disable the repositories you want to install. Use SPACE for selecting/deselecting";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

        };
    };

public:
    PresenterRepository(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

