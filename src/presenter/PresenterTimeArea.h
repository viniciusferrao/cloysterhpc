/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERTIMEZONEAREA_H_
#define CLOYSTERHPC_PRESENTERTIMEZONEAREA_H_

#include "Presenter.h"

class PresenterTimeArea : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Time settings - Area choice";

        struct Timezone {
            static constexpr const char* question
                = "Choose you local timezone area";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterTimeArea(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERTIMEZONEAREA_H_
