//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 5/25/23.
//

#include <cloysterhpc/presenter/PresenterNodesOperationalSystem.h>

namespace fs = std::filesystem;

PresenterNodesOperationalSystem::PresenterNodesOperationalSystem(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    std::list<std::string> distroNames;
    distroNames.emplace_back("Red Hat Enterprise Linux");
    distroNames.emplace_back("AlmaLinux");
    distroNames.emplace_back("Rocky Linux");
    distroNames.emplace_back("Oracle Linux");

    std::map<std::string, OS::Distro> distros;
    distros["Red Hat Enterprise Linux"] = OS::Distro::RHEL;
    distros["AlmaLinux"] = OS::Distro::AlmaLinux;
    distros["Rocky Linux"] = OS::Distro::Rocky;
    distros["Oracle Linux"] = OS::Distro::OL;

    // Download remote ISO image or use local image

    auto downloadIso = false;

    downloadIso = m_view->yesNoQuestion(Messages::title,
        Messages::OperationalSystemDownloadIso::FirstStage::question,
        Messages::OperationalSystemDirectoryPath::help);

    if (downloadIso) {

        // Download remote ISO
        auto distroToDownload = m_view->listMenu(Messages::title,
            Messages::OperationalSystemDownloadIso::SecondStage::question,
            distroNames,
            Messages::OperationalSystemDownloadIso::SecondStage::help);

        auto selectedDistro = distros.find(distroToDownload);

        std::string distroDownloadURL = "https://mirror.versatushpc.com.br/";
        std::string isoName = "OracleLinux-R8-U7-x86_64-dvd.iso";

        switch (selectedDistro->second) {
            case OS::Distro::RHEL:
                // @TODO
                distroDownloadURL += "";
                isoName = "";
                break;
            case OS::Distro::OL:
                // Waiting for mirror.versatushpc.com.br fix
                distroDownloadURL
                    += "oracle/iso/OracleLinux-R8-U8-x86_64-dvd.iso";
                isoName = "OracleLinux-R8-U8-x86_64-dvd.iso";
                break;
            case OS::Distro::Rocky:
                distroDownloadURL
                    += "rocky/linux/8.8/isos/x86_64/Rocky-8.8-x86_64-dvd1.iso";
                isoName = "Rocky-8.8-x86_64-dvd1.iso";
                break;
            case OS::Distro::AlmaLinux:
                distroDownloadURL += "almalinux/almalinux/8.8/isos/x86_64/"
                                     "AlmaLinux-8.8-x86_64-dvd.iso";
                isoName = "AlmaLinux-8.8-x86_64-dvd.iso";
                break;
        }

        //@TODO Implement newt GUI progress bar
        cloyster::runCommand(
            fmt::format("wget -NP /root {}", distroDownloadURL));

        m_model->setDiskImage(fmt::format("/root/{}", isoName));
        LOG_DEBUG("Selected ISO: {}", fmt::format("/root/{}", isoName));

    } else {
        // Operational system directory path selection

        auto isoDirectoryPath
            = std::to_array<std::pair<std::string, std::string>>(
                { { Messages::OperationalSystemDirectoryPath::field,
                    "/mnt/iso" } });

        isoDirectoryPath = m_view->fieldMenu(Messages::title,
            Messages::OperationalSystemDirectoryPath::question,
            isoDirectoryPath, Messages::OperationalSystemDirectoryPath::help);

        LOG_DEBUG(
            "ISO directory path set to {}", isoDirectoryPath.data()->second);

        // Operational system distro selection

        auto selectedDistroName = m_view->listMenu(Messages::title,
            Messages::OperationalSystemDistro::question, distroNames,
            Messages::OperationalSystemDistro::help);

        auto selectedDistro = distros.find(selectedDistroName);

        // Operational system iso selection

        std::list<std::string> isos;

        for (const auto& entry :
            fs::directory_iterator(isoDirectoryPath.data()->second)) {
            if (entry.path().string().ends_with("iso")) {
                auto formattedIsoName = entry.path().string().erase(
                    entry.path().string().find(isoDirectoryPath.data()->second),
                    isoDirectoryPath.data()->second.length() + 1);

                switch (selectedDistro->second) {
                    case OS::Distro::RHEL:
                        if (formattedIsoName.find("rhel") != std::string::npos)
                            isos.emplace_back(formattedIsoName);
                        break;
                    case OS::Distro::OL:
                        if (formattedIsoName.find("OracleLinux")
                            != std::string::npos)
                            isos.emplace_back(formattedIsoName);
                        break;
                    case OS::Distro::Rocky:
                        if (formattedIsoName.find("Rocky") != std::string::npos)
                            isos.emplace_back(formattedIsoName);
                        break;
                    case OS::Distro::AlmaLinux:
                        if (formattedIsoName.find("AlmaLinux")
                            != std::string::npos)
                            isos.emplace_back(formattedIsoName);
                        break;
                }
            }
        }

        auto selectedIso = m_view->listMenu(Messages::title,
            Messages::OperationalSystem::question, isos,
            Messages::OperationalSystem::help);

        m_model->setDiskImage(
            fmt::format("{}/{}", isoDirectoryPath.data()->second, selectedIso));
        LOG_DEBUG("Selected ISO: {}",
            fmt::format("{}/{}", isoDirectoryPath.data()->second, selectedIso));
    }
}