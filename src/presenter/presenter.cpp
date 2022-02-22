//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "presenterNetwork.h"

Presenter::Presenter(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model)
                     : m_model(model), m_view(view) {

#if 0 // Welcome messages
    welcomeMessage();
    LOG_TRACE("Welcome message displayed");

    installInstructions();
    LOG_TRACE("Install instructions displayed");
#endif

#if 0 // Set general settings
    auto generalSettings = std::to_array<
            std::pair<std::string, std::string>>({
                    {"Cluster Name", "cl0yst3r"},
                    {"Company Name", "Profiterolis Corp"},
                    {"Administrator e-mail", "root@example.com"}
            });

    generalSettings = m_view->fieldMenu("General cluster settings", "Fill the required data about your new cluster",
                      generalSettings, "");
    m_model->setName(generalSettings[0].second);
    LOG_INFO("Set cluster name: {}", m_model->getName());
    m_model->setCompanyName(generalSettings[1].second);
    LOG_INFO("Set cluster company name: {}", m_model->getCompanyName());
    m_model->setAdminMail(generalSettings[2].second);
    LOG_INFO("Set cluster admin e-email: {}", m_model->getAdminMail());
#endif

#if 0 // Timezone and locale support
    m_model->setTimezone(timezoneSelection(m_model->getTimezone().getAvailableTimezones()));
    // TODO: Horrible call; getTimezone() two times? Srsly?
    LOG_TRACE("Timezone set to: {}", m_model->getTimezone().getTimezone());

    // TODO: Fix the interface hack to show only one time "time server"
    auto timeservers = std::to_array<
            std::pair<std::string, std::string>>({
                    {"Time server", "0.br.pool.ntp.org"},
                    {"", "1.br.pool.ntp.org"},
                    {"", "2.br.pool.ntp.org"}
            });

    // TODO: Set timeservers
    timeservers = m_view->fieldMenu("Time server settings",
                                    "Add or change the list of available time servers",
                                    timeservers, "");
//    m_model->getTimezone().setTimeservers(timeservers);
//    LOG_INFO("Timeservers set to {}", m_model->getTimezone().getTimeservers());

    // TODO: Get locales from the system
    auto locales = std::to_array<std::string_view>(
            {"en_US.UTF-8", "pt_BR.UTF-8", "C"});

    m_model->setLocale(localeSelection(locales));
    LOG_TRACE("Locale set to: {}", m_model->getLocale());

    // TODO: Get rid of aux
    auto aux = networkHostnameSelection(std::to_array<
            std::pair<std::string, std::string>>({
                    {"Hostname", "headnode"},
                    {"Domain name", "example.com"}
            }));

    //std::vector<std::string> aux = networkHostnameSelection({"Hostname", "Domain name"});
    m_model->getHeadnode().setHostname(aux[0].second);
    LOG_TRACE("Returned hostname: {}", aux[0].second);
    LOG_ASSERT(aux[0].second == m_model->getHeadnode().getHostname(),
               "Failed setting hostname");

    m_model->setDomainName(aux[1].second);
    LOG_TRACE("Hostname set to: {}", m_model->getHeadnode().getHostname());
    LOG_TRACE("Domain name set to: {}", m_model->getDomainName());
    LOG_TRACE("FQDN: {}", m_model->getHeadnode().getFQDN());
#endif

#if 0 // Boot target on headnode selection
    m_model->getHeadnode().setBootTarget(
            magic_enum::enum_cast<Headnode::BootTarget>(
                m_view->listMenu(
                    "General settings",
                    "Select the boot target for the headnode",
                    magic_enum::enum_names<Headnode::BootTarget>(),
                    "No help")
            ).value()
    );
    LOG_INFO("{} boot target set on headnode",
             magic_enum::enum_name<Headnode::BootTarget>(
                     m_model->getHeadnode().getBootTarget()));
#endif

#if 0 // Lambda test
    [this](std::vector<std::string> aux) -> void {
        aux = networkHostnameSelection({"Hostname", "Domain name"});
        m_model->getHeadnode().setHostname(aux[0]);
        m_model->setDomainName(aux[1]);
    };
#endif

#if 1 // Networking
    // TODO: Under development
    try {
        PresenterNetwork external(view, model);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::External),
                 ex.what());
    }

    try {
        PresenterNetwork management(view, model, Network::Profile::Management);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 magic_enum::enum_name(Network::Profile::Management),
                 ex.what());
    }
#endif

#if 1 // Infiniband support
    // TODO: Infiniband class? Detect if IB is available (fetch ib0)
    if (m_view->yesNoQuestion("Infiniband Network", "Do you have an Infiniband Fabric available?", "No help")) {

        m_model->setOFED(magic_enum::enum_cast<Cluster::OFED>(
                m_view->listMenu(MSG_TITLE_INFINIBAND_SETTINGS,
                                 MSG_INFINIBAND_SETTINGS,
                                 magic_enum::enum_names<Cluster::OFED>(),
                                 MSG_INFINIBAND_SETTINGS_HELP)).value()
                );
        LOG_INFO("Set OFED stack as: {}", magic_enum::enum_name<Cluster::OFED>(m_model->getOFED()));

        try {
            PresenterNetwork application(view, model, Network::Profile::Application, Network::Type::Infiniband);
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to add {} network: {}",
                     magic_enum::enum_name(Network::Profile::Application),
                     ex.what());
        }
    }
#endif

#if 1 // Compute nodes formation details
    m_view->message("We will now gather information to fill your compute nodes data");

    // TODO: Placeholder data
    auto fields = std::to_array<
            std::pair<std::string, std::string>>({
                    {"Prefix", "n"},
                    {"Padding", "2"},
                    {"Compute node first IP", "172.31.22.45"},
                    {"Compute node root password", "p@ssw0rd"},
                    {"ISO path of Node OS", "/root/iso/rhel-8.5-dvd1.iso"}
            });

    retry:
    fields = m_view->fieldMenu("Node Settings", MSG_NODE_SETTINGS, fields,
                                MSG_NODE_SETTINGS_HELP);

    for (const auto& field : fields) {
        if (field.first == "Prefix") {
            if (std::isalpha(field.second[0] == false)) {
                m_view->message("Prefix must start with a letter");
                goto retry;
            }
        }

        if (field.first == "Padding") {
            if (boost::lexical_cast<size_t>(field.second) > 3) {
                m_view->message("We can only support up to 1000 nodes");
                goto retry;
            }
        }
    }

    // TODO: Encapsulate
    m_model->nodePrefix = fields[0].second;
    m_model->nodePadding = boost::lexical_cast<size_t>(fields[1].second);
    m_model->nodeStartIP = fields[2].second;
    m_model->nodeRootPassword = fields[3].second;
    m_model->setISOPath(fields[4].second);
#endif

#if 1 // Compute nodes details
    auto nodes = std::to_array<
            std::pair<std::string, std::string>>({
                    {"Racks", "2"},
                    {"Nodes", "5"},
                    {"Node start number", "7"},
                    {"Node base name", "n"}
            });

    m_view->fieldMenu("Node Settings",
                      "Fill the required node information data",
                      nodes,
                      "No help");
#endif

#if 1 // Queue System
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

#if 1 // Mail system
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

void Presenter::welcomeMessage() {
    m_view->message(MSG_WELCOME);
}

void Presenter::installInstructions() {
    m_view->okCancelMessage(MSG_GUIDED_INSTALL);
}

std::string Presenter::timezoneSelection(const std::vector<std::string>& timezones) {
    return m_view->listMenu(MSG_TITLE_TIME_SETTINGS,
                            MSG_TIME_SETTINGS_TIMEZONE, timezones,
                            MSG_TIME_SETTINGS_TIMEZONE_HELP);
}

template<size_t N>
std::string Presenter::localeSelection(const std::array<std::string_view, N>& locales)
{
    return std::string{m_view->listMenu(MSG_TITLE_LOCALE_SETTINGS,
                            MSG_LOCALE_SETTINGS_LOCALE, locales,
                            MSG_LOCALE_SETTINGS_LOCALE_HELP)};
}

template<size_t N>
std::array<std::pair<std::string, std::string>, N>
Presenter::networkHostnameSelection(const std::array<std::pair<std::string, std::string>, N>& entries)
{
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_HOSTID, entries,
                             MSG_NETWORK_SETTINGS_HOSTID_HELP);
}

//std::vector<std::string>
//Presenter::networkHostnameSelection(const std::vector<std::string>& entries)
//{
//    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
//                             MSG_NETWORK_SETTINGS_HOSTID, entries,
//                             MSG_NETWORK_SETTINGS_HOSTID_HELP);
//}
