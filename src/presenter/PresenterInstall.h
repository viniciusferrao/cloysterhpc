//
// Created by Vinícius Ferrão on 24/11/21.
//

#ifndef CLOYSTERHPC_PRESENTERINSTALL_H
#define CLOYSTERHPC_PRESENTERINSTALL_H

#include "Presenter.h"

#include "../include/magic_enum.hpp"
#include <boost/lexical_cast.hpp>

class PresenterInstall : public Presenter {
public:
    PresenterInstall(std::unique_ptr<Cluster>& model,
                     std::unique_ptr<Newt>& view);
};

#endif //CLOYSTERHPC_PRESENTERINSTALL_H
