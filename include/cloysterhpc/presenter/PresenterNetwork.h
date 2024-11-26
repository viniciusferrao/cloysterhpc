/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNETWORK_H_
#define CLOYSTERHPC_PRESENTERNETWORK_H_

#include <cloysterhpc/presenter/Presenter.h>

#include <boost/asio.hpp>
#include <cloysterhpc/cluster.h>
#include <cloysterhpc/network.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/view/newt.h>

#include <magic_enum.hpp>

#include <memory>
#include <utility>

struct NetworkCreatorData {
    Network::Profile profile;
    Network::Type type;
    std::string interface;
    std::string address;
    std::string subnetMask;
    std::string gateway;
    std::string name;
    std::vector<boost::asio::ip::address> domains;
};

class NetworkCreator {
private:
    std::vector<NetworkCreatorData> m_networks;

    bool checkIfProfileExists(Network::Profile profile);

public:
    bool addNetworkInformation(NetworkCreatorData&& data);

    bool checkIfInterfaceRegistered(std::string_view interface);

    void saveNetworksToModel(Cluster& model);
};

class PresenterNetwork : public Presenter {
private:
    std::unique_ptr<Network> m_network;

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

    void createNetwork(NetworkCreatorData& ncd);

public:
    PresenterNetwork(std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view, NetworkCreator& nc,
        Network::Profile profile = Network::Profile::External,
        Network::Type type = Network::Type::Ethernet);
};

#endif // CLOYSTERHPC_PRESENTERNETWORK_H_
