//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "presenterNetwork.h"
#include "../services/log.h"

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
            std::pair<std::string, std::variant<std::string, unsigned>>>({
                    {"Cluster Name", "cl0yst3r"},
                    {"Company Name", "Profiterolis Corp"},
                    {"Administrator e-mail", "root@example.com"}
            });

    generalSettings = m_view->fieldMenu("General cluster settings", "Fill the required data about your new cluster",
                      generalSettings, "");
    m_model->setName(get<std::string>(generalSettings[0].second));
    LOG_INFO("Set cluster name: {}", m_model->getName());
    m_model->setCompanyName(get<std::string>(generalSettings[1].second));
    LOG_INFO("Set cluster company name: {}", m_model->getCompanyName());
    m_model->setAdminMail(get<std::string>(generalSettings[2].second));
    LOG_INFO("Set cluster admin e-email: {}", m_model->getAdminMail());
#endif

#if 0 // Timezone and locale support
    m_model->setTimezone(timezoneSelection(m_model->getTimezone().getAvailableTimezones()));
    // TODO: Horrible call; getTimezone() two times? Srsly?
    LOG_TRACE("Timezone set to: {}", m_model->getTimezone().getTimezone());

    // TODO: Fix the interface hack to show only one time "time server"
    auto timeservers = std::to_array<
            std::pair<std::string, std::variant<std::string, unsigned>>>({
                    {"Time server", "0.br.pool.ntp.org"},
                    {"", "1.br.pool.ntp.org"},
                    {"", "2.br.pool.ntp.org"}
            });

    timeservers = m_view->fieldMenu("Time server settings",
                                    "Add or change the list of available time servers",
                                    timeservers, "");
//    m_model->getTimezone().setTimeservers(timeservers);
//    LOG_INFO("Timeservers set to {}", m_model->getTimezone().getTimeservers());

    // TODO: Get locales from the system
    m_model->setLocale(localeSelection({"en_US.UTF-8", "pt_BR.UTF-8", "C"}));
    LOG_TRACE("Locale set to: {}", m_model->getLocale());

    // TODO: Get rid of aux
    std::vector<std::string> aux = networkHostnameSelection({"Hostname", "Domain name"});
    m_model->getHeadnode().setHostname(aux[0]);
    LOG_TRACE("Returned hostname: {}", aux[0]);
    LOG_ASSERT(aux[0] == m_model->getHeadnode().getHostname(),
               "Failed setting hostname");

    m_model->setDomainName(aux[1]);
    LOG_TRACE("Hostname set to: {}\n", m_model->getHeadnode().getHostname());
    LOG_TRACE("Domain name set to: {}\n", m_model->getDomainName());
    LOG_TRACE("FQDN: {}\n", m_model->getHeadnode().getFQDN());
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

#if 0 // Networking
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

#if 0 // Infiniband support
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

#if 0 // Compute nodes formation details
    m_view->message("We will now gather information to fill your compute nodes data");

    // TODO: Placeholder data
    auto fields = std::to_array<
            std::pair<std::string, std::variant<std::string, size_t>>>({
//    const std::vector<std::pair<std::string,std::string>> fields = {
                    {"Prefix","n"},
                    {"Padding", 2ul}, // TODO: should be integer on .second?
                    {"Compute node first IP", "172.31.22.45"},
                    {"Compute node root password","p@ssw0rd"},
                    {"ISO path of Node OS","/root/iso/rhel-8.5-dvd1.iso"}
            });

    retry:
    fields = m_view->fieldMenu("Node Settings", MSG_NODE_SETTINGS, fields,
                                MSG_NODE_SETTINGS_HELP);

    for (const auto& field : fields) {
        if (field.first == "Prefix") {
            if (std::isalpha(std::get<std::string>(field.second)[0]) == false) {
                m_view->message("Prefix must start with a letter");
                goto retry;
            }
        }

        // FIXME: I'm throwing on std::get<size_t>
        if (field.first == "Padding") {
            //if (std::get<size_t>(field.second) > 3ul) {
            if (std::stoul(std::get<std::string>(field.second)) > 3) {
                m_view->message("We can only support up to 1000 nodes");
                goto retry;
            }
        }
    }

    // TODO: Encapsulate
    m_model->nodePrefix = std::get<std::string>(fields[0].second);
    m_model->nodePadding = std::stoul(std::get<std::string>(fields[1].second));
    m_model->nodeStartIP = std::get<std::string>(fields[2].second);
    m_model->nodeRootPassword = std::get<std::string>(fields[3].second);
    m_model->setISOPath(std::get<std::string>(fields[4].second));
#endif

#if 0 // Compute nodes details
    // FIXME: std::variant is irrelevant due to a bug on newt::fieldMenu.
    auto nodes = std::to_array<
            std::pair<std::string, std::variant<std::string, size_t>>>({
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

#if 0 // Queue System
    m_model->setQueueSystem(
            magic_enum::enum_cast<QueueSystem::Kind>(
                    m_view->listMenu(MSG_TITLE_QUEUE_SYSTEM_SETTINGS,
                                     MSG_QUEUE_SYSTEM_SETTINGS,
                                     magic_enum::enum_names<QueueSystem::Kind>(),
                                     MSG_QUEUE_SYSTEM_SETTINGS_HELP)).value());

    // TODO: Placeholder data
    auto fieldsSLURM = std::to_array<
            std::pair<std::string, std::variant<std::string, size_t>>>({
                    {"Partition Name","execution"}
            });
    auto listPBS = std::to_array<std::string_view>(
            {"Shared", "Scatter"}
    );

    if (m_model->getQueueSystem()) {
        switch (m_model->getQueueSystem()->getKind()) {
            case QueueSystem::Kind::None:
                break;

            case QueueSystem::Kind::SLURM:
                // TODO: Set the gathered data
                m_view->fieldMenu(MSG_TITLE_SLURM_SETTINGS,
                                  MSG_SLURM_SETTINGS,
                                  fieldsSLURM,
                                  MSG_SLURM_SETTINGS_HELP);
                break;

            case QueueSystem::Kind::PBS:
                // TODO: Set the gathered data
                m_view->listMenu(MSG_TITLE_PBS_SETTINGS,
                                 MSG_PBS_SETTINGS,
                                 listPBS,
                                 MSG_PBS_SETTINGS_HELP);
                break;
        }
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

std::string Presenter::localeSelection(const std::vector<std::string>& locales)
{
    return m_view->listMenu(MSG_TITLE_LOCALE_SETTINGS,
                            MSG_LOCALE_SETTINGS_LOCALE, locales,
                            MSG_LOCALE_SETTINGS_LOCALE_HELP);
}

std::vector<std::string>
Presenter::networkHostnameSelection(const std::vector<std::string>& entries)
{
    return m_view->fieldMenu(MSG_TITLE_NETWORK_SETTINGS,
                             MSG_NETWORK_SETTINGS_HOSTID, entries,
                             MSG_NETWORK_SETTINGS_HOSTID_HELP);
}
