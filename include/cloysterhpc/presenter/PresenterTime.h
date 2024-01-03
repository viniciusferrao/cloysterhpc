/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERTIMEZONE_H_
#define CLOYSTERHPC_PRESENTERTIMEZONE_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterTime : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Time and clock settings";

        struct Timezone {
            static constexpr const auto question = "Choose your local timezone";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct Timeservers {
            static constexpr const auto question
                = "Add or change the list of available time servers";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
            static constexpr const auto field = "Time server(s)";
        };
    };

public:
    PresenterTime(std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERTIMEZONE_H_
