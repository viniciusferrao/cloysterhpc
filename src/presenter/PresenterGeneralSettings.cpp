/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterGeneralSettings.h>

PresenterGeneralSettings::PresenterGeneralSettings(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    // Generic settings
    auto generalSettings = std::to_array<std::pair<std::string, std::string>>(
        { { Messages::General::clusterName, "" },
            { Messages::General::companyName, "" },
            { Messages::General::adminEmail, "" } });

    generalSettings = m_view->fieldMenu(Messages::title,
        Messages::General::question, generalSettings, Messages::General::help);

    std::size_t i { 0 };
    m_model->setName(generalSettings[i++].second);
    LOG_DEBUG("Set cluster name: {}", m_model->getName())

    m_model->setCompanyName(generalSettings[i++].second);
    LOG_DEBUG("Set cluster company name: {}", m_model->getCompanyName())

    m_model->setAdminMail(generalSettings[i++].second);
    LOG_DEBUG("Set cluster admin e-email: {}", m_model->getAdminMail())

    // Boot target
    m_model->getHeadnode().setBootTarget(
        magic_enum::enum_cast<Headnode::BootTarget>(
            m_view->listMenu(Messages::title, Messages::BootTarget::question,
                magic_enum::enum_names<Headnode::BootTarget>(),
                Messages::BootTarget::help))
            .value());
    LOG_DEBUG("{} boot target set on headnode",
        magic_enum::enum_name<Headnode::BootTarget>(
            m_model->getHeadnode().getBootTarget()));
}
