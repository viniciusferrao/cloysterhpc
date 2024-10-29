/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
#define CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_

#include <cloysterhpc/presenter/Presenter.h>
#include <optional>

using PresenterNodesVersionCombo
    = std::tuple<int, int, OS::Arch>; // major, minor

class PresenterNodesOperationalSystem : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Nodes operational system settings";

        struct OperationalSystemDownloadIso {
            struct FirstStage {
                static constexpr const auto question
                    = "You want to download a ISO for your node?";
                static constexpr const auto help
                    = "Choose 'YES' if you want to download a new one or 'NO' "
                      "if you already have an ISO.";
            };
            struct SecondStage {
                static constexpr const auto question
                    = "Choose an ISO to download";
                static constexpr const auto help
                    = Presenter::Messages::Placeholder::help;
            };
            struct Progress {
                static constexpr const auto download
                    = "Downloading ISO from {0}\nSource: {1}";
            };
        };

        struct OperationalSystemDirectoryPath {
            static constexpr const auto question
                = "Inform the directory where your operational system images "
                  "are";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
            static constexpr const auto field = "Path to ISOs directory:";

            static constexpr const auto nonExistent
                = "The specified directory do not exist";
        };

        struct OperationalSystemDistro {
            static constexpr const auto question
                = "Choose your operational system distro";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct OperationalSystemVersion {
            static constexpr const auto question = "Choose your distro version";
            static constexpr const auto rhelError
                = "Unfortunately, we do not support downloading Red Hat "
                  "Enterprise Linux yet.\n"
                  "Please download the ISO yourself and put in an appropriate "
                  "location.";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct OperationalSystem {
            static constexpr const auto question
                = "Choose your operational system ISO";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };
    };

    std::optional<PresenterNodesVersionCombo> selectVersion(OS::Distro distro);
    std::string getDownloadURL(
        OS::Distro distro, PresenterNodesVersionCombo version);

public:
    PresenterNodesOperationalSystem(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
