/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PresenterWelcome.h"

PresenterWelcome::PresenterWelcome(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    m_view->message(Messages::Welcome::message);
    LOG_DEBUG("Welcome message displayed");
}
