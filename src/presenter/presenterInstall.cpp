//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenterInstall.h"
#include "presenterWelcome.h"
#include "PresenterInstructions.h"
#include "PresenterGeneralSettings.h"
#include "PresenterTime.h"
#include "PresenterLocale.h"
#include "PresenterHostId.h"
#include "presenterNetwork.h"
#include "PresenterInfiniband.h"
#include "PresenterNodes.h"

PresenterInstall::PresenterInstall(std::unique_ptr<Cluster>& model,
                                   std::unique_ptr<Newt>& view)
                                   : Presenter(model, view)
{

#if 1 // Welcome messages
    Call<PresenterWelcome>();
    Call<PresenterInstructions>();
#endif

#if 1 // Set general settings
    Call<PresenterGeneralSettings>();
#endif

#if 1 // Timezone and locale support
    Call<PresenterTime>();
    Call<PresenterLocale>();
#endif

#if 1 // Hostname and domain
    Call<PresenterHostId>();
#endif

#if 1 // Networking
    // TODO: Under development
    //  * Add it to a loop where it asks to the user which kind of network we
    //  should add, while the operator says it's done adding networks. We remove
    //  the lazy network{1,2} after that.

    try {
        Call<PresenterNetwork>();
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::External), ex.what());
    }

    try {
        Call<PresenterNetwork>(Network::Profile::Management);
        //PresenterNetwork network(model, view, Network::Profile::Management);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::Management), ex.what());
    }
#endif

#if 1 // Infiniband support
    Call<PresenterInfiniband>();
#endif

#if 1 // Compute nodes formation details
    Call<PresenterNodes>();
#endif

#if 0 // Queue System
    m_model->setQueueSystem(
            magic_enum::enum_cast<QueueSystem::Kind>(
                    m_view->listMenu(MSG_TITLE_QUEUE_SYSTEM_SETTINGS,
                                     MSG_QUEUE_SYSTEM_SETTINGS,
                                     magic_enum::enum_names<QueueSystem::Kind>(),
                                     MSG_QUEUE_SYSTEM_SETTINGS_HELP)).value());

    // TODO: Placeholder data
    auto fieldsSLURM = std::to_array<
            std::pair<std::string, std::string>>({
                    {"Partition Name","execution"}
            });
    auto listPBS = std::to_array<std::string_view>(
            {"Shared", "Scatter"}
    );

    if (auto& queue = m_model->getQueueSystem()) {
        switch (queue.value()->getKind()) {
            case QueueSystem::Kind::None: {
                __builtin_unreachable();
            }

            case QueueSystem::Kind::SLURM: {
                fieldsSLURM = m_view->fieldMenu(MSG_TITLE_SLURM_SETTINGS,
                                                MSG_SLURM_SETTINGS,
                                                fieldsSLURM,
                                                MSG_SLURM_SETTINGS_HELP);

                const auto &slurm = dynamic_cast<SLURM*>(queue.value().get());
                slurm->setDefaultQueue(fieldsSLURM[0].second);
                LOG_INFO("Set SLURM default queue: {}", slurm->getDefaultQueue());

                break;
            }

            case QueueSystem::Kind::PBS: {
                const auto &execution = m_view->listMenu(
                        MSG_TITLE_PBS_SETTINGS,
                        MSG_PBS_SETTINGS,
                        listPBS,
                        MSG_PBS_SETTINGS_HELP);

                const auto &pbs = dynamic_cast<PBS*>(queue.value().get());
                pbs->setExecutionPlace(
                        magic_enum::enum_cast<PBS::ExecutionPlace>(execution).value());
                LOG_INFO("Set PBS Execution Place: {}",
                        magic_enum::enum_name<PBS::ExecutionPlace>(pbs->getExecutionPlace()));

                break;
            }
        }
    }
#endif

#if 0 // Mail system
    if (m_view->yesNoQuestion(
            MSG_TITLE_POSTFIX_SETTINGS,
            MSG_POSTFIX_ENABLE,
            MSG_POSTFIX_ENABLE_HELP)) {

        m_model->setMailSystem(
                magic_enum::enum_cast<Postfix::Profile>(
                        m_view->listMenu(MSG_TITLE_POSTFIX_SETTINGS,
                                         MSG_POSTFIX_SETTINGS_PROFILE,
                                         magic_enum::enum_names<Postfix::Profile>(),
                                         MSG_POSTFIX_SETTINGS_PROFILE_HELP)).value());

        auto& mailSystem = m_model->getMailSystem().value();
        const auto& mailSystemProfile = mailSystem.getProfile();

        LOG_INFO("Enabled Postfix with profile: {}",
                 magic_enum::enum_name<Postfix::Profile>(mailSystemProfile));

        switch (mailSystemProfile) {
            case Postfix::Profile::Local: {
                break;
            }

            case Postfix::Profile::Relay: {
                auto fields = std::to_array<
                        std::pair<std::string, std::string>>({
                                {"Hostname of the MTA", ""},
                                {"Port", "25"}
                        });

                fields = m_view->fieldMenu(MSG_TITLE_POSTFIX_SETTINGS,
                                           MSG_POSTFIX_RELAY_SETTINGS,
                                           fields,
                                           MSG_POSTFIX_RELAY_SETTINGS_HELP);

                mailSystem.setHostname(fields[0].second);
                mailSystem.setPort(boost::lexical_cast<uint16_t>(fields[1].second));

                LOG_INFO("Set Postfix Relay: {}:{}",
                         mailSystem.getHostname().value(),
                         mailSystem.getPort().value());

                break;
            }

            case Postfix::Profile::SASL: {
                auto fields = std::to_array<
                        std::pair<std::string, std::string>>({
                                {"Hostname of the MTA", ""},
                                {"Port", ""},
                                {"Username", ""},
                                {"Password", ""}
                        });

                fields = m_view->fieldMenu(MSG_TITLE_POSTFIX_SETTINGS,
                                           MSG_POSTFIX_SASL_SETTINGS,
                                           fields,
                                           MSG_POSTFIX_SASL_SETTINGS_HELP);

                mailSystem.setHostname(fields[0].second);
                mailSystem.setPort(boost::lexical_cast<uint16_t>(fields[1].second));
                mailSystem.setUsername(fields[2].second);
                mailSystem.setPassword(fields[3].second);

                LOG_INFO("Set Postfix SASL: {}:{}\nUsername: {} | Password: {}",
                         mailSystem.getHostname().value(),
                         mailSystem.getPort().value(),
                         mailSystem.getUsername().value(),
                         mailSystem.getPassword().value());

                break;
            }
        }

    } else {
        LOG_INFO("Postfix wasn't enabled");
    }
#endif

    // Destroy the view since we don't need it anymore
    m_view.reset();
}
