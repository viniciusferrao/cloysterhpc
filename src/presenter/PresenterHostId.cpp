//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterHostId.h"

PresenterHostId::PresenterHostId(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    // Should this be a std::vector instead?
    // std::vector<std::string> aux = networkHostnameSelection({"Hostname", "Domain name"});
    const auto& entries = std::to_array<std::pair<std::string, std::string>>({
            {Messages::hostname, "headnode"},
            {Messages::domainName, "example.com"}
    });

    const auto& answers = m_view->fieldMenu(Messages::title,
                                            Messages::question, entries,
                                            Messages::help);

    m_model->getHeadnode().setHostname(answers[0].second);
    // FIXME: This assert never happens since the setter may throw
//    LOG_TRACE("Returned hostname: {}", answers[0].second);
//    LOG_ASSERT(answers[0].second == m_model->getHeadnode().getHostname(),
//               "Failed setting hostname");

    m_model->setDomainName(answers[1].second);
    LOG_TRACE("Hostname set to: {}", m_model->getHeadnode().getHostname());
    LOG_TRACE("Domain name set to: {}", m_model->getDomainName());
    LOG_TRACE("FQDN: {}", m_model->getHeadnode().getFQDN());
}
