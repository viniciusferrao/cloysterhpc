/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_
#define CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_

#include "Presenter.h"

class PresenterGeneralSettings : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "General cluster settings";

        struct General {
            static constexpr const char* question
                = "Fill the required data about your new cluster";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;

            static constexpr const char* clusterName = "Cluster Name";
            static constexpr const char* companyName = "Company Name";
            static constexpr const char* adminEmail = "Administrator e-mail";
        };

        struct BootTarget {
            static constexpr const char* question
                = "Select the boot target for the headnode";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterGeneralSettings(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERGENERALSETTINGS_H_
