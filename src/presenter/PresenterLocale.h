//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERLOCALE_H
#define CLOYSTERHPC_PRESENTERLOCALE_H

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

#endif //CLOYSTERHPC_PRESENTERLOCALE_H
