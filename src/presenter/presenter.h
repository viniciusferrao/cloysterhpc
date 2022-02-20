//
// Created by Vinícius Ferrão on 24/11/21.
//

#ifndef CLOYSTERHPC_PRESENTER_H
#define CLOYSTERHPC_PRESENTER_H

#include "../view/newt.h"
#include "../cluster.h"
#include "../services/log.h"

#include "../include/magic_enum.hpp"

#include <memory>
#include <variant>
#include <fmt/format.h>
#include <string_view>

#include <boost/lexical_cast.hpp>

// TODO: Pure virtual
class Presenter {
private:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    void welcomeMessage();
    void installInstructions();
    std::string timezoneSelection(const std::vector<std::string>&);

    // TODO: Try to return an string_view instead
    template<size_t N>
    std::string localeSelection(const std::array<std::string_view, N>& locales);

//    std::vector<std::string> networkHostnameSelection(const std::vector<std::string>&);

    // TODO: Organize this code
    template<size_t N>
    std::array<std::pair<std::string, std::string>, N>
    networkHostnameSelection(const std::array<std::pair<std::string, std::string>, N>& entries);

public:
    Presenter(std::unique_ptr<Newt>& view,
              std::unique_ptr<Cluster>& model);


};

#endif //CLOYSTERHPC_PRESENTER_H
