//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 5/25/23.
//

#include "PresenterNodesOperationalSystem.h"

namespace fs = std::filesystem;

PresenterNodesOperationalSystem::PresenterNodesOperationalSystem(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    // Operational system directory path selection

    auto isoDirectoryPath = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::OperationalSystemDirectoryPath::field, "/mnt/iso" } });

    isoDirectoryPath = m_view->fieldMenu(Messages::title,
        Messages::OperationalSystemDirectoryPath::question, isoDirectoryPath,
        Messages::OperationalSystemDirectoryPath::help);

    LOG_DEBUG("ISO directory path set to {}", isoDirectoryPath.data()->second);

    // Operational system iso selection

    std::list<std::string> isos;

    for (const auto& entry :
        fs::directory_iterator(isoDirectoryPath.data()->second)) {
        if (entry.path().string().ends_with("iso")) {
            auto formattedIsoName = entry.path().string().erase(
                entry.path().string().find(isoDirectoryPath.data()->second),
                isoDirectoryPath.data()->second.length() + 1);
            isos.emplace_back(formattedIsoName);
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