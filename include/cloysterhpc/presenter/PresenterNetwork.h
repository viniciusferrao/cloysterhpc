/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNETWORK_H_
#define CLOYSTERHPC_PRESENTERNETWORK_H_

#include <cloysterhpc/presenter/Presenter.h>

#include <cloysterhpc/cluster.h>
#include <cloysterhpc/network.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/view/newt.h>

#include <magic_enum.hpp>

#include <memory>
#include <utility>

class PresenterNetwork : public Presenter {
private:
    std::unique_ptr<Network> m_network;
    Connection m_connection;

    struct Messages {
        static constexpr const auto title = "Network Settings";
        static constexpr const auto errorInsufficient
            = "Not enough interfaces!\nYou need at least two separate cards: "
              "one internal and one external";

        struct Interface {
            static std::string formatQuestion(
                Network::Type type, Network::Profile profile)
            {
                return fmt::format("Select your {} ({}) network interface",
                    magic_enum::enum_name(profile),
                    magic_enum::enum_name(type));
            }

            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct Details {
            static constexpr const auto question
                = "Fill the required network details";
            static constexpr const auto help
                = Presenter::Messages::Placeholder::help;
        };

        struct IP {
            static constexpr const auto address = "IP Address";
            static constexpr const auto subnetMask = "Subnet Mask";
            static constexpr const auto gateway = "Gateway";
        };

        struct Domain {
            static constexpr const auto name = "Domain name";
            static constexpr const auto servers = "Nameservers";
        };

#ifndef NDEBUG
        struct Debug {
            static constexpr const auto attributes
                = "The following network attributes were detected";
        };
#endif
    };

    template <typename T>
    std::string networkInterfaceSelection(const T& interfaces)
    {
        return m_view->listMenu(Messages::title,
            Messages::Interface::formatQuestion(
                m_network->getType(), m_network->getProfile())
                .c_str(),
            interfaces, Messages::Interface::help);
    }

    // Tested with T = std::array<std::pair<std::string, std::string>, N>
    template <typename T> T networkAddress(const T& fields)
    {
        return m_view->fieldMenu(Messages::title, Messages::Details::question,
            fields, Messages::Details::help);
    }

#ifndef NDEBUG
    // TODO: Better implementation
    // Tested with T = std::array<std::pair<std::string, std::string>, N>
    template <typename T> void networkConfirmation(const T& pairs)
    {
        m_view->okCancelMessage(Messages::Debug::attributes, pairs);
    }
#endif

    void createNetwork();

public:
    PresenterNetwork(std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view,
        Network::Profile profile = Network::Profile::External,
        Network::Type type = Network::Type::Ethernet);
};

#endif // CLOYSTERHPC_PRESENTERNETWORK_H_
