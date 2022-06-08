/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERLOCALE_H_
#define CLOYSTERHPC_PRESENTERLOCALE_H_

#include "Presenter.h"

class PresenterLocale : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Locale settings";
        static constexpr const char* question = "Pick your default locale";
        static constexpr const char* help = Presenter::Messages::Placeholder::help;
    };

public:
    PresenterLocale(std::unique_ptr<Cluster>& model,
                    std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERLOCALE_H_
