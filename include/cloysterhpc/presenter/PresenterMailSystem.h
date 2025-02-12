/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERMAILSYSTEM_H_
#define CLOYSTERHPC_PRESENTERMAILSYSTEM_H_

#include <cloysterhpc/presenter/Presenter.h>

namespace cloyster::presenter {
class PresenterMailSystem : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Mail system settings";
        static constexpr const auto question
            = "Do you want to enable Postfix mail system?";
        static constexpr const auto help
            = Presenter::Messages::Placeholder::help;

        struct Profile {
            static constexpr const auto question
                = "Choose a profile for mail delivery";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct Relay {
            static constexpr const auto question
                = "Enter the destination MTA information to relay messages";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto hostname = "Hostname of the MTA";
            static constexpr const auto port = "Port";
        };

        struct SASL {
            static constexpr const auto question
                = "Enter the mail server and user information to deliver "
                  "messages";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto hostname = "Hostname of the MTA";
            static constexpr const auto port = "Port";
            static constexpr const auto username = "Username";
            static constexpr const auto password = "Password";
        };
    };

public:
    PresenterMailSystem(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

}; 

#endif // CLOYSTERHPC_PRESENTERMAILSYSTEM_H_
