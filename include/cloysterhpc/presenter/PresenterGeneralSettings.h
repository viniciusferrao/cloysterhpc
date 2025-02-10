/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_
#define CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_

#include <cloysterhpc/presenter/Presenter.h>


class PresenterGeneralSettings : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "General cluster settings";

        struct General {
            static constexpr const auto question
                = "Fill the required data about your new cluster";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto clusterName = "Cluster Name";
            static constexpr const auto companyName = "Company Name";
            static constexpr const auto adminEmail = "Administrator e-mail";
        };

        struct BootTarget {
            static constexpr const auto question
                = "Select the boot target for the headnode";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterGeneralSettings(
        std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_
