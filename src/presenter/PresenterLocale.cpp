/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterLocale.h>

PresenterLocale::PresenterLocale(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    // TODO: Get locales from the system
    //    auto availableLocales = m_model->getLocale()
    //                                    .getAvailableLocales();

    // FIXME: For now we will only support those two locales
    const auto& locales
        = std::to_array<std::string_view>({ "en_US.UTF-8", "pt_BR.UTF-8" });

    const auto& selectedLocale = m_view->listMenu(
        Messages::title, Messages::question, locales, Messages::help);

    m_model->setLocale(selectedLocale);
    LOG_DEBUG("Locale set to: {}", m_model->getLocale());
}
