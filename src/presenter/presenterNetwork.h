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
    std::vector<std::pair<std::string, std::string>> m_networkDetails;

    // Helper methods
    template<class T>
    void addNetworkDetail(const std::string& key, const T& value);

    std::string networkInterfaceSelection(const std::vector<std::string>&);
    std::vector<std::string> networkAddress(const std::vector<std::string>&);
    std::vector<std::pair<std::string,std::string>> networkAddress(const std::vector<std::pair<std::string,std::string>>& fields);
#ifndef _NDEBUG_
    void networkConfirmation(const std::vector<std::pair<std::string, std::string>>& pairs);
#endif

public:
    PresenterNetwork(std::unique_ptr<Newt>& view,
                     std::unique_ptr<Cluster>& model,
                     Network::Profile profile = Network::Profile::External,
                     Network::Type type = Network::Type::Ethernet);
};


#endif //CLOYSTERHPC_PRESENTERNETWORK_H
