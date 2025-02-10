/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERLOCALE_H_
#define CLOYSTERHPC_PRESENTERLOCALE_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterLocale : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Locale settings";
        static constexpr const auto question = "Pick your default locale";
        static constexpr const auto help
            = Presenter::Messages::Placeholder::help;
    };

public:
    PresenterLocale(
        std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERLOCALE_H_
