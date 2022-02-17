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

// TODO: Pure virtual
class Presenter {
private:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    void welcomeMessage();
    void installInstructions();
    std::string timezoneSelection(const std::vector<std::string>&);
    std::string localeSelection(const std::vector<std::string>&);
    std::vector<std::string> networkHostnameSelection(const std::vector<std::string>&);

public:
    Presenter(std::unique_ptr<Newt>& view,
              std::unique_ptr<Cluster>& model);


};

#endif //CLOYSTERHPC_PRESENTER_H
