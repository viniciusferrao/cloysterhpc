/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNODES_H_
#define CLOYSTERHPC_PRESENTERNODES_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterNodes : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Compute nodes settings";
        static constexpr const auto message
            = "We will now gather information to fill your compute nodes data";

        struct Nodes {
            static constexpr const auto question
                = "Enter the compute nodes information";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto prefix = "Prefix";
            static constexpr const auto padding = "Padding";
            static constexpr const auto startIP = "Compute node first IP";
            static constexpr const auto rootPassword
                = "Compute node root password";
            static constexpr const auto confirmRootPassword
                = "Confirm compute node root password";
            static constexpr const auto DiskImagePath
                = "Disk image path of node OS";
        };

        struct Error {
            static constexpr const auto prefixLetter
                = "Prefix must start with a letter";
            static constexpr const auto paddingMax
                = "We can only support up to 1000 nodes";
        };

        struct Quantity {
            static constexpr const auto question
                = "Enter the compute nodes quantity information";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;

            static constexpr const auto racks = "Racks";
            static constexpr const auto nodes = "Nodes";
            static constexpr const auto startNumber = "Node start number";
        };

        struct MAC {
            static constexpr const auto question
                = "Enter the MAC address for node";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };
    };

public:
    PresenterNodes(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERNODES_H_
