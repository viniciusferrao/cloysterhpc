//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERGENERALSETTINGS_H
#define CLOYSTERHPC_PRESENTERGENERALSETTINGS_H

#include "presenter.h"

class PresenterGeneralSettings : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "General cluster settings";

        struct General {
            static constexpr const char* question = "Fill the required data about your new cluster";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;

            static constexpr const char* clusterName = "Cluster Name";
            static constexpr const char* companyName = "Company Name";
            static constexpr const char* adminEmail = "Administrator e-mail";
        };

        struct BootTarget {
            static constexpr const char* question = "Select the boot target for the headnode";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterGeneralSettings(std::unique_ptr<Cluster>& model,
                             std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERGENERALSETTINGS_H
