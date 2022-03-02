//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERWELCOME_H
#define CLOYSTERHPC_PRESENTERWELCOME_H

#include "presenter.h"

class PresenterWelcome : public Presenter {
private:
    struct Messages {
        struct Welcome {
            static constexpr const char *message =
                "Welcome to the guided installer of CloysterHPC!\n\nLorem ipsum dolor " \
                "sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt " \
                "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud " \
                "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. " \
                "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum " \
                "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non " \
                "proident, sunt in culpa qui officia deserunt mollit anim id est laborum." \
                "\n";
        };
    };

public:
    PresenterWelcome(std::unique_ptr<Cluster>& model,
                     std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERWELCOME_H
