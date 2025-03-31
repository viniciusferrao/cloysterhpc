/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/services/osservice.h>
#include <cloysterhpc/presenter/PresenterLocale.h>

namespace cloyster::presenter {

PresenterLocale::PresenterLocale(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    auto osservice = cloyster::Singleton<cloyster::services::IOSService>::get();
    auto availableLocales = osservice->getAvailableLocales();

    const auto& selectedLocale = m_view->listMenu(
        Messages::title, Messages::question, availableLocales, Messages::help);

    m_model->setLocale(selectedLocale);
    LOG_DEBUG("Locale set to: {}", selectedLocale)
}

};
