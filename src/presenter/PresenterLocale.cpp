/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterLocale.h>

PresenterLocale::PresenterLocale(
    std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    auto availableLocales = m_model->getLocale().getAvailableLocales();

    const auto& selectedLocale = m_view->listMenu(
        Messages::title, Messages::question, availableLocales, Messages::help);

    m_model->setLocale(selectedLocale);
    LOG_DEBUG("Locale set to: {}", m_model->getLocale().getLocale())
}
