/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERTIMEZONE_H_
#define CLOYSTERHPC_PRESENTERTIMEZONE_H_

#include "Presenter.h"

class PresenterTime : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Time and clock settings";

        struct Timezone {
            static constexpr const char* question = "Choose you local timezone";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };

        struct Timeservers {
            static constexpr const char* question
                = "Add or change the list of available time servers";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
            static constexpr const char* field = "Time server(s)";
        };
    };

public:
    PresenterTime(std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERTIMEZONE_H_
