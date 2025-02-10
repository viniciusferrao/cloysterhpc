/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterHostId.h>

PresenterHostId::PresenterHostId(
    std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    while (true) {

        // Should this be a std::vector instead?
        // std::vector<std::string> aux = networkHostnameSelection({"Hostname",
        // "Domain name"});
        const auto& entries
            = std::to_array<std::pair<std::string, std::string>>(
                { { Messages::hostname, m_model->getHeadnode().getHostname() },
                    { Messages::domainName, m_model->getDomainName() } });

        const auto& answers = m_view->fieldMenu(
            Messages::title, Messages::question, entries, Messages::help);

        m_model->getHeadnode().setHostname(answers[0].second);
        m_model->setDomainName(answers[1].second);

        auto hostnameValidation = m_model->getHeadnode().validateHostname();
        auto fqdnValidation = m_model->getHeadnode().validateFQDN();

        if (!hostnameValidation.has_value()) {
            m_view->message(Messages::title,
                fmt::format(
                    "Hostname validation error: {}", hostnameValidation.error())
                    .c_str());
            continue;
        }

        if (!fqdnValidation.has_value()) {
            m_view->message(Messages::title,
                fmt::format(
                    "Domain validation error: {}", fqdnValidation.error())
                    .c_str());
            continue;
        }

        break;
    }

    LOG_DEBUG("Hostname set to: {}", m_model->getHeadnode().getHostname())
    LOG_DEBUG("Domain name set to: {}", m_model->getDomainName())
    LOG_DEBUG("FQDN: {}", m_model->getHeadnode().getFQDN())
}
