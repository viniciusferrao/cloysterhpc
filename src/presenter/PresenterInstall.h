/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERINSTALL_H_
#define CLOYSTERHPC_PRESENTERINSTALL_H_

#include "Presenter.h"

#include <boost/lexical_cast.hpp>
#include <magic_enum.hpp>

class PresenterInstall : public Presenter {
public:
    PresenterInstall(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERINSTALL_H_
