//
// Created by Vinícius Ferrão on 07/02/22.
//

#ifndef CLOYSTERHPC_PRESENTERNETWORK_H
#define CLOYSTERHPC_PRESENTERNETWORK_H

#include "../view/newt.h"
#include "../cluster.h"
#include "../network.h"
#include "../services/log.h"

#include "../include/magic_enum.hpp"

#include <memory>
#include <utility>

class PresenterNetwork {
private:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    template<typename T>
    std::string networkInterfaceSelection(const T& interfaces);

    template<size_t N>
    std::array<std::pair<std::string,std::string>, N>
    networkAddress(const std::array<std::pair<std::string,std::string>, N>& fields);

#ifndef _NDEBUG_
    template<size_t N>
    void networkConfirmation(const std::array<std::pair<std::string, std::string>, N>& pairs);
#endif

public:
    PresenterNetwork(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model,
                     Network::Profile profile = Network::Profile::External,
                     Network::Type type = Network::Type::Ethernet);
};


#endif //CLOYSTERHPC_PRESENTERNETWORK_H
