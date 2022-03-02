//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTERNODES_H
#define CLOYSTERHPC_PRESENTERNODES_H

#include "presenter.h"

class PresenterNodes : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Compute nodes settings";
        static constexpr const char* message = "We will now gather information to fill your compute nodes data";

        struct Nodes {
            static constexpr const char* question = "Enter the compute nodes information";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;

            static constexpr const char* prefix = "Prefix";
            static constexpr const char* padding = "Padding";
            static constexpr const char* startIP = "Compute node first IP";
            static constexpr const char* rootPassword = "Compute node root password";
            static constexpr const char* ISOPath = "ISO path of Node OS";
        };

        struct Error {
            static constexpr const char* prefixLetter = "Prefix must start with a letter";
            static constexpr const char* paddingMax = "We can only support up to 1000 nodes";
        };

        struct Quantity {
            static constexpr const char* question = "Enter the compute nodes quantity information";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;

            static constexpr const char* racks = "Racks";
            static constexpr const char* nodes = "Nodes";
            static constexpr const char* startNumber = "Node start number";
        };

    };

public:
    PresenterNodes(std::unique_ptr<Cluster>& model,
                   std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERNODES_H
