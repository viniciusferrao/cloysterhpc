//
// Created by Vinícius Ferrão on 24/11/21.
//

#include "presenter.h"
#include "presenterNetwork.h"
#include "../services/log.h"

Presenter::Presenter(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model)
                     : m_model(model), m_view(view) {

#if 0
    welcomeMessage();
    LOG_TRACE("Welcome message displayed");

    installInstructions();
    LOG_TRACE("Install instructions displayed");

    // Set general settings
    // std::vector<std::pair<std::string_view,std::string_view>> generalSettings = {
    std::vector<std::pair<std::string,std::string>> generalSettings = {
            {"Cluster Name", ""},
            {"Company Name", ""},
            {"Administrator e-mail", ""}
    };
    generalSettings = m_view->fieldMenu("General cluster settings", "Fill the required data about your new cluster",
                      generalSettings, "");
    m_model->setName(generalSettings[0].second);
    LOG_INFO("Set cluster name: {}", m_model->getName());
    m_model->setCompanyName(generalSettings[1].second);
    LOG_INFO("Set cluster company name: {}", m_model->getCompanyName());
    m_model->setAdminMail(generalSettings[2].second);
    LOG_INFO("Set cluster admin e-email: {}", m_model->getAdminMail());

    // Timezone and locale support
    m_model->setTimezone(timezoneSelection(m_model->getTimezone().getAvailableTimezones()));
    // TODO: Horrible call; getTimezone() two times? Srsly?
    LOG_TRACE("Timezone set to: {}", m_model->getTimezone().getTimezone());

    // TODO: Fix the interface hack to show only one time "time server"
    std::vector<std::pair<std::string,std::string>> timeservers = {
            {"Time server", "0.br.pool.ntp.org"},
            {"", "1.br.pool.ntp.org"},
            {"", "2.br.pool.ntp.org"}
    };
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

    // Boot target on headnode selection
    // TODO: Enum handling (magic_enum)
    m_model->getHeadnode().setTarget(Headnode::Target::Text);
            m_view->listMenu(
            "General settings",
            "Select the boot target for the headnode",
            {"Text", "Graphical"},
            "No help");
    LOG_INFO("{} target set on headnode")//, m_model->(getHeadnode().getTarget());
#endif

#if 0
    [this](std::vector<std::string> aux) -> void {
        aux = networkHostnameSelection({"Hostname", "Domain name"});
        m_model->getHeadnode().setHostname(aux[0]);
        m_model->setDomainName(aux[1]);
    };
#endif

#if 0
    // TODO: Under development
    try {
        PresenterNetwork external(view, model);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 Network::getProfileString.at(Network::Profile::External),
                 ex.what());
    }

    try {
        PresenterNetwork management(view, model, Network::Profile::Management);
    } catch (const std::exception& ex) {
        LOG_WARN("Failed to add {} network: {}",
                 Network::getProfileString.at(Network::Profile::Management),
                 ex.what());
    }
#endif

#if 0
    // Infiniband support
    // TODO: Infiniband class? Detect if IB is available (fetch ib0)
    if (m_view->yesNoQuestion("Infiniband Network", "Do you have an Infiniband Fabric available?", "No help")) {

        const auto& ibStacks = {
                Cluster::getOFEDString.at(Cluster::OFED::Inbox),
                Cluster::getOFEDString.at(Cluster::OFED::Mellanox),
                Cluster::getOFEDString.at(Cluster::OFED::Oracle)
        };

        // TODO: Boost.bimap to m_model->setOFED();
        m_view->listMenu(MSG_TITLE_INFINIBAND_SETTINGS, MSG_INFINIBAND_SETTINGS, ibStacks, MSG_INFINIBAND_SETTINGS_HELP);
        //m_model->setOFED(m_view->listMenu(
        //        MSG_TITLE_INFINIBAND_SETTINGS, MSG_INFINIBAND_SETTINGS, ibStacks, MSG_INFINIBAND_SETTINGS_HELP));

        try {
            PresenterNetwork application(view, model, Network::Profile::Application, Network::Type::Infiniband);
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to add {} network: {}",
                     Network::getProfileString.at(Network::Profile::Application),
                     ex.what());
        }
    }
#endif

#if 0
    // Compute nodes details
    m_view->message("We will now gather information to fill your compute nodes data");

    // TODO: Placeholder data
    const std::vector<std::pair<std::string,std::string>> fields = {
            {"Prefix","n"},
            {"Padding","2"}, // TODO: should be integer on .second?
            {"Compute node first IP","1"},
            {"Compute node root password","p@ssw0rd"},
            {"ISO path of Node OS","/root/iso/rhel-8.5-dvd1.iso"}
    };

    retry:
    const auto& nodeData = m_view->fieldMenu("Node Settings", MSG_NODE_SETTINGS, fields, MSG_NODE_SETTINGS_HELP);

    for (const auto& nd : nodeData) {
        if (nd.first == "Prefix")
            if (std::isalpha(nd.second[0]) == false) {
                m_view->message(nullptr, "Prefix must start with a letter");
                goto retry;
            }

        if (nd.first == "Padding") {
            if (std::stoul(nd.second) > 3) {
                m_view->message(nullptr, "We can only support up to 1000 nodes");
                goto retry;
            }
        }
    }

    // TODO: Encapsulate
    m_model->nodePrefix = nodeData[0].second;
    m_model->nodePadding = nodeData[1].second;
    m_model->nodeStartIP = nodeData[2].second;
    m_model->nodeRootPassword = nodeData[3].second;
    m_model->setISOPath(nodeData[4].second);
#endif

#if 1
//    std::unordered_map<std::string, std::string> nodes;
//    nodes.reserve(4);
//    nodes.emplace("Racks");
//    nodes.emplace("Nodes");
//    nodes.emplace("Node start number");
//    nodes.emplace("Node base name");
    std::vector<std::pair<std::string, std::variant<std::string, unsigned>>> nodes = {
            {"Racks", 1u},
            {"Nodes", 5u},
            {"Node start number", 1u},
            {"Node base name", "n"}
    };

//    nodes.reserve(4);
//    nodes.emplace_back("Racks", 1u);
//    nodes.emplace_back("Nodes", 1u);
//    nodes.emplace_back("Node start number", 1u);
//    nodes.emplace_back("Node base name", "n");

    m_view->fieldMenu("Node Settings",
                      "Fill the required node information data",
                      nodes,
                      "No help");

#endif

#if 0
    // Queue System
    // TODO: Template this. This calls looks just like OFED selection.
    const auto& queueSystems = {
            QueueSystem::getKindString.at(QueueSystem::Kind::None),
            QueueSystem::getKindString.at(QueueSystem::Kind::SLURM),
            QueueSystem::getKindString.at(QueueSystem::Kind::PBS)
    };

    const auto& queueSystem = m_view->listMenu(MSG_TITLE_QUEUE_SYSTEM_SETTINGS,
                                               MSG_QUEUE_SYSTEM_SETTINGS,
                                               queueSystems,
                                               MSG_QUEUE_SYSTEM_SETTINGS_HELP);

    // TODO: Better handling of this case statement
    if (queueSystem == "SLURM")
        m_model->setQueueSystem(QueueSystem::Kind::SLURM);
    else if (queueSystem == "PBS")
        m_model->setQueueSystem(QueueSystem::Kind::PBS);
//    else
//        m_model->setQueueSystem(QueueSystem::Kind::None);

    // TODO: Placeholder data
    const std::vector<std::pair<std::string,std::string>> fieldsSLURM = {
            {"Partition Name","execution"}
    };
    const std::vector<std::string> listPBS = {"Shared", "Scatter"};

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
