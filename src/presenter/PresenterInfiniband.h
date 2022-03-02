//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERINFINIBAND_H
#define CLOYSTERHPC_PRESENTERINFINIBAND_H

#include "Presenter.h"
#include "PresenterNetwork.h"

class PresenterInfiniband : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Infiniband settings";
        static constexpr const char* question = "Do you have an Infiniband Fabric available?";
        static constexpr const char* help = Presenter::Messages::Placeholder::help;

        struct OFED {
            static constexpr const char* question = "Choose the desired Infiniband stack";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterInfiniband(std::unique_ptr<Cluster>& model,
                        std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERINFINIBAND_H
