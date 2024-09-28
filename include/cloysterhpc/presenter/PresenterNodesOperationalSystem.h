/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
#define CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_

#include <cloysterhpc/presenter/Presenter.h>

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

        struct OperationalSystem {
            static constexpr const auto question
                = "Choose your operational system ISO";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterNodesOperationalSystem(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
