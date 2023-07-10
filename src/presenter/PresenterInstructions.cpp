/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterInstructions.h>

PresenterInstructions::PresenterInstructions(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    m_view->message(Messages::Instructions::message);
    LOG_DEBUG("Install instructions displayed");
}
