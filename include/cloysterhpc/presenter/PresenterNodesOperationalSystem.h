//
// Created by Lucas Gracioso <contact@lbgracioso.net> on 5/25/23.
//

#ifndef CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
#define CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterNodesOperationalSystem : public Presenter {
private:
    struct Messages {
        static constexpr const char* title
            = "Nodes operational system settings";

        struct OperationalSystemDownloadIso {
            struct FirstStage {
                static constexpr const char* question
                    = "You want to download a ISO for your node?";
                static constexpr const char* help
                    = "Choose 'YES' if you want to download a new one or 'NO' "
                      "if you already have an ISO.";
            };
            struct SecondStage {
                static constexpr const char* question
                    = "Choose an ISO to download";
                static constexpr const char* help
                    = Presenter::Messages::Placeholder::help;
            };
        };

        struct OperationalSystemDirectoryPath {
            static constexpr const char* question
                = "Inform the directory where your operational system images "
                  "are";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
            static constexpr const char* field = "Path to ISOs directory:";
        };

        struct OperationalSystemDistro {
            static constexpr const char* question
                = "Choose your operational system distro";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };

        struct OperationalSystem {
            static constexpr const char* question
                = "Choose your operational system ISO";
            static constexpr const char* help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterNodesOperationalSystem(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERNODESOPERATIONALSYSTEM_H_
