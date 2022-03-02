//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERINSTRUCTIONS_H
#define CLOYSTERHPC_PRESENTERINSTRUCTIONS_H

#include "presenter.h"

class PresenterInstructions : public Presenter {
private:
    struct Messages {
        struct Instructions {
            static constexpr const char* message =
                "We will start the installation phase now.\n\nYou will be guided through " \
                "a series of questions about your HPC cluster so we can configure " \
                "everything accordingly to your needs. You're free to cancel the process " \
                "at any given time.\n";
        };
    };

public:
    PresenterInstructions(std::unique_ptr<Cluster>& model,
                          std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERINSTRUCTIONS_H
