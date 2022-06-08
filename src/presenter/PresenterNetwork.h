/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERNETWORK_H_
#define CLOYSTERHPC_PRESENTERNETWORK_H_

#include "Presenter.h"

#include "../view/newt.h"
#include "../cluster.h"
#include "../network.h"
#include "../services/log.h"

#include "../include/magic_enum.hpp"

#include <memory>
#include <utility>

class PresenterNetwork : public Presenter {
private:
    std::unique_ptr<Network> m_network;
    Connection m_connection;

    struct Messages {
        static constexpr const char* title = "Network Settings";

        // TODO: Find a way to express which interface you are dealing with:
        //       "Select your <external> network interface", for example.
        struct Interface {
            static constexpr const char* question = "Select your network interface";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;
        };

        struct Details {
            static constexpr const char* question = "Fill the required network details";
            static constexpr const char* help = Presenter::Messages::Placeholder::help;
        };

        struct IP {
            static constexpr const char* address = "IP Address";
            static constexpr const char* subnetMask = "Subnet Mask";
            static constexpr const char* network = "Network Address";
            static constexpr const char* gateway = "Gateway";
        };

        struct Domain {
            static constexpr const char* name = "Domain name";
            static constexpr const char* servers = "Nameservers";
        };

#ifndef NDEBUG
        struct Debug {
            static constexpr const char* attributes = "The following network attributes were detected";
        };
#endif
    };

    template<typename T>
    std::string networkInterfaceSelection(const T& interfaces)
    {
        return m_view->listMenu(
                Messages::title,
                Messages::Interface::question,
                interfaces,
                Messages::Interface::help);
    }

    // Tested with T = std::array<std::pair<std::string, std::string>, N>
    template<typename T>
    T networkAddress(const T& fields)
    {
        return m_view->fieldMenu(
                Messages::title,
                Messages::Details::question,
                fields,
                Messages::Details::help);
    }

#ifndef NDEBUG
    // TODO: Better implementation
    // Tested with T = std::array<std::pair<std::string, std::string>, N>
    template <typename T>
    void networkConfirmation(const T& pairs)
    {
        m_view->okCancelMessage(Messages::Debug::attributes, pairs);
    }
#endif

    void createNetwork();

public:
    PresenterNetwork(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view,
        Network::Profile profile = Network::Profile::External,
        Network::Type type = Network::Type::Ethernet);
};


#endif // CLOYSTERHPC_PRESENTERNETWORK_H_
