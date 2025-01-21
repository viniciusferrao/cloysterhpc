/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/presenter/PresenterNodesOperationalSystem.h>
#include <cloysterhpc/services/log.h>

#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <filesystem>
#include <fmt/args.h>
#include <fmt/core.h>
#include <ranges>
#include <string_view>

namespace fs = std::filesystem;

std::string PresenterNodesOperationalSystem::getDownloadURL(
    OS::Distro distro, PresenterNodesVersionCombo version)
{
    auto [majorVersion, minorVersion, arch] = version;

    fmt::dynamic_format_arg_store<fmt::format_context> store;
    store.push_back(fmt::arg("arch", magic_enum::enum_name(arch)));
    store.push_back(fmt::arg("major", majorVersion));
    store.push_back(fmt::arg("minor", minorVersion));

    switch (distro) {
        case OS::Distro::RHEL:
            throw std::runtime_error(
                "We does not support RHEL ISO download yet!");
        case OS::Distro::OL:
            return fmt::vformat("https://yum.oracle.com/ISOS/OracleLinux/"
                                "OL{major}/u{minor}/{arch}/"
                                "OracleLinux-R{major}-U{minor}-{arch}-dvd.iso",
                store);
        case OS::Distro::Rocky:
            return fmt::vformat(
                "https://download.rockylinux.org/pub/rocky/{major}/"
                "isos/{arch}/Rocky-{major}.{minor}-{arch}-dvd.iso",
                store);
        case OS::Distro::AlmaLinux:
            return fmt::vformat("https://repo.almalinux.org/almalinux/"
                                "{major}.{minor}/isos/{arch}/"
                                "AlmaLinux-{major}.{minor}-{arch}-dvd.iso",
                store);
    }

    return "?";
}

const std::unordered_map<OS::Distro, std::vector<PresenterNodesVersionCombo>>
    version_map({ { OS::Distro::AlmaLinux,
                      {
                          { 9, 4, OS::Arch::x86_64 },
                          { 9, 4, OS::Arch::ppc64le },
                          { 9, 3, OS::Arch::x86_64 },
                          { 9, 3, OS::Arch::ppc64le },
                          { 9, 2, OS::Arch::x86_64 },
                          { 9, 2, OS::Arch::ppc64le },
                      } },
        { OS::Distro::Rocky,
            {
                { 9, 4, OS::Arch::x86_64 },
                { 9, 4, OS::Arch::ppc64le },
                { 9, 3, OS::Arch::x86_64 },
                { 9, 3, OS::Arch::ppc64le },
                { 9, 2, OS::Arch::x86_64 },
                { 9, 2, OS::Arch::ppc64le },
            } },
        { OS::Distro::OL,
            {
                { 9, 4, OS::Arch::x86_64 },
                { 9, 3, OS::Arch::x86_64 },
                { 9, 2, OS::Arch::x86_64 },
            } } });

std::optional<PresenterNodesVersionCombo>
PresenterNodesOperationalSystem::selectVersion(OS::Distro distro)
{
    if (distro == OS::Distro::RHEL)
        return std::nullopt;

    auto nameiter = version_map.at(distro)
        | std::views::transform([](const PresenterNodesVersionCombo& c) {
              auto [maj, min, arch] = c;
              return fmt::format(
                  "{}.{} ({})", maj, min, magic_enum::enum_name(arch));
          });

    std::vector<std::string> versions;
    std::copy(
        std::begin(nameiter), std::end(nameiter), std::back_inserter(versions));

    auto version_to_download = m_view->listMenu(Messages::title,
        Messages::OperationalSystemVersion::question, versions,
        Messages::OperationalSystemVersion::help);

    if (auto findit
        = std::find(versions.begin(), versions.end(), version_to_download);
        findit != versions.end()) {
        const auto currentver = std::distance(versions.begin(), findit);
        const auto& vdistro = version_map.at(distro);
        LOG_ASSERT(currentver >= 0, "currentver is negative");
        return std::make_optional<PresenterNodesVersionCombo>(
            vdistro[static_cast<size_t>(currentver)]);
    }

    return std::nullopt;
}

PresenterNodesOperationalSystem::PresenterNodesOperationalSystem(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    std::vector<std::string> distroNames;
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

        auto versioncombo = selectVersion(selectedDistro->second);
        std::string distroDownloadURL
            = getDownloadURL(selectedDistro->second, *versioncombo);
        std::string isoName
            = distroDownloadURL.substr(distroDownloadURL.find_last_of('/'));

        //@TODO Implement newt GUI progress bar
        auto command = cloyster::runCommandIter(
            fmt::format("wget -NP /root {}", distroDownloadURL),
            cloyster::Stream::Stderr);

        auto desc = fmt::format(
            Messages::OperationalSystemDownloadIso::Progress::download,
            selectedDistro->first, distroDownloadURL);
        m_view->progressMenu(Messages::title, desc.c_str(), std::move(command),
            [&](cloyster::CommandProxy& cmd) -> std::optional<double> {
                auto out = cmd.getline();
                if (!out) {
                    return std::nullopt;
                }
                std::string line = *out;

                // If we have a line like ERROR 404: Not Found
                // this means, obviously, that we did not found the URL.
                if (line.contains("ERROR 404: Not Found")) {
                    LOG_ERROR("URL {} not found", distroDownloadURL);
                    return std::nullopt;
                }

                // Line example
                //  <<<338950K .......... .......... .......... ..........
                //  ..........  3% 31.8M 10m40s>>

                // TODO: (on the progress bar) maybe allow altering some menu
                // parameters (like the text)
                std::vector<std::string> slots;

                boost::split(slots, line, boost::is_any_of("\t\r "),
                    boost::token_compress_on);

                if (slots.size() <= 6) {
                    return std::make_optional(0.0);
                }

                auto num = slots[6].substr(0, slots[6].find_first_of('%'));

                try {
                    return std::make_optional(boost::lexical_cast<double>(num));
                } catch (boost::bad_lexical_cast&) {
                    return std::make_optional(0.0);
                }
            });

        m_model->setDiskImage(fmt::format("/root/{}", isoName));
        LOG_DEBUG("Selected ISO: {}", fmt::format("/root/{}", isoName))

    } else {
        // Operational system directory path selection

        auto isoDirectoryPath
            = std::to_array<std::pair<std::string, std::string>>(
                { { Messages::OperationalSystemDirectoryPath::field,
                    "/mnt/iso" } });

        while (true) {
            isoDirectoryPath = m_view->fieldMenu(Messages::title,
                Messages::OperationalSystemDirectoryPath::question,
                isoDirectoryPath,
                Messages::OperationalSystemDirectoryPath::help);

            if (std::filesystem::exists(isoDirectoryPath.data()->second)) {
                break;
            }

            m_view->message(Messages::title,
                Messages::OperationalSystemDirectoryPath::nonExistent);
        }

        LOG_DEBUG(
            "ISO directory path set to {}", isoDirectoryPath.data()->second);

        // Operational system distro selection

        auto selectedDistroName = m_view->listMenu(Messages::title,
            Messages::OperationalSystemDistro::question, distroNames,
            Messages::OperationalSystemDistro::help);

        auto selectedDistro = distros.find(selectedDistroName);

        // Operational system iso selection

        auto isoRoot = isoDirectoryPath.data()->second;
        std::vector<std::string> isos;

        for (const auto& entry : fs::directory_iterator(isoRoot)) {
            if (entry.path().string().ends_with("iso")) {
                auto formattedIsoName = entry.path().filename().string();

                isos.emplace_back(formattedIsoName);

                // TODO: this detection method is not reliable
                // The right way should be to mount the ISO and check inside of
                // it.

                /**
                   [root@cloyster home]# mount -o loop /opt/iso/cloyster-iso.iso
                 /mnt mount: /mnt: WARNING: device write-protected, mounted
                 read-only. [root@cloyster home]# ls /mnt AppStream  BaseOS  EFI
                 images  isolinux  LICENSE  media.repo  TRANS.TBL [root@cloyster
                 home]# less /mnt/media.repo [InstallMedia] name=Rocky Linux 8.8
                   mediaid=None
                   metadata_expire=-1
                   gpgcheck=0
                   cost=500
                 **/
                switch (selectedDistro->second) {
                    case OS::Distro::RHEL:
                        if (formattedIsoName.contains("rhel")) {
                            isos.emplace_back(formattedIsoName);
                        }
                        break;
                    case OS::Distro::OL:
                        if (formattedIsoName.contains("OracleLinux")) {
                            isos.emplace_back(formattedIsoName);
                        }
                        break;
                    case OS::Distro::Rocky:
                        if (formattedIsoName.contains("Rocky")) {
                            isos.emplace_back(formattedIsoName);
                        }
                        break;
                    case OS::Distro::AlmaLinux:
                        if (formattedIsoName.contains("AlmaLinux")) {
                            isos.emplace_back(formattedIsoName);
                        }
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
