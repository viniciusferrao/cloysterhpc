//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERMAILSYSTEM_H
#define CLOYSTERHPC_PRESENTERMAILSYSTEM_H

#include "Presenter.h"

class PresenterMailSystem : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Mail system settings";
        static constexpr const char* question = "Do you want to enable Postfix mail system?";
        static constexpr const char* help = Presenter::Messages::Placeholder::help;

        struct Profile {
            static constexpr const char* question = "Choose a profile for mail delivery";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;
        };

        struct Relay {
            static constexpr const char* question = "Enter the destination MTA information to relay messages";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;

            static constexpr const char* hostname = "Hostname of the MTA";
            static constexpr const char* port = "Port";
        };

        struct SASL {
            static constexpr const char* question = "Enter the mail server and user information to deliver messages";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;

            static constexpr const char* hostname = "Hostname of the MTA";
            static constexpr const char* port = "Port";
            static constexpr const char* username = "Username";
            static constexpr const char* password = "Password";
        };
    };

public:
    PresenterMailSystem(std::unique_ptr<Cluster>& model,
                             std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERMAILSYSTEM_H
