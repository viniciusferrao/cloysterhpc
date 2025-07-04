/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/presenter/PresenterMailSystem.h>

using cloyster::services::Postfix;

namespace cloyster::presenter {
PresenterMailSystem::PresenterMailSystem(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    if (m_view->yesNoQuestion(
            Messages::title, Messages::question, Messages::help)) {

        Postfix::Profile mailSystemProfile
            = cloyster::utils::enums::ofStringOpt<Postfix::Profile>(
                m_view->listMenu(Messages::title, Messages::Profile::question,
                    cloyster::utils::enums::toStrings<Postfix::Profile>(),
                    Messages::Profile::help))
                  .value();
        m_model->setMailSystem(mailSystemProfile);
        auto mailSystem = m_model->getMailSystem().value(); // copy here

        LOG_DEBUG("Enabled Postfix with profile: {}",
            cloyster::utils::enums::toString<Postfix::Profile>(
                mailSystemProfile));

        switch (mailSystemProfile) {
            case Postfix::Profile::Local: {
                break;
            }

            case Postfix::Profile::Relay: {
                auto fields
                    = std::to_array<std::pair<std::string, std::string>>(
                        { { Messages::Relay::hostname, "" },
                            { Messages::Relay::port, "25" } });

                fields = m_view->fieldMenu(Messages::title,
                    Messages::Relay::question, fields, Messages::Relay::help);

                std::size_t i { 0 };
                mailSystem.setHostname(fields[i++].second);
                mailSystem.setPort(
                    boost::lexical_cast<uint16_t>(fields[i++].second));

                LOG_DEBUG("Set Postfix Relay: {}:{}",
                    mailSystem.getHostname().value(),
                    mailSystem.getPort().value());

                break;
            }

            case Postfix::Profile::SASL: {
                auto fields
                    = std::to_array<std::pair<std::string, std::string>>(
                        { { Messages::SASL::hostname, "" },
                            { Messages::SASL::port, "587" },
                            { Messages::SASL::username, "" },
                            { Messages::SASL::password, "" } });

                fields = m_view->fieldMenu(Messages::title,
                    Messages::SASL::question, fields, Messages::SASL::help);

                std::size_t i { 0 };
                mailSystem.setHostname(fields[i++].second);
                mailSystem.setPort(
                    boost::lexical_cast<uint16_t>(fields[i++].second));
                mailSystem.setUsername(fields[i++].second);
                mailSystem.setPassword(fields[i++].second);

                LOG_DEBUG(
                    "Set Postfix SASL: {}:{}\nUsername: {} | Password: {}",
                    mailSystem.getHostname().value(),
                    mailSystem.getPort().value(),
                    mailSystem.getUsername().value(),
                    mailSystem.getPassword().value());

                break;
            }
        }

    } else {
        LOG_DEBUG("Postfix wasn't enabled")
    }
}

};
