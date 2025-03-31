/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/presenter/PresenterInfiniband.h>

#include <algorithm>

namespace cloyster::presenter {

PresenterInfiniband::PresenterInfiniband(std::unique_ptr<Cluster>& model,
    std::unique_ptr<Newt>& view, NetworkCreator& nc)
    : Presenter(model, view)
{

    auto interfaces = Connection::fetchInterfaces();
    if (std::find(interfaces.begin(), interfaces.end(), "ib0")
        == interfaces.end()) {
        LOG_WARN("No Infiniband interfaces found.")
        return;
    }

    // TODO: Infiniband class? Detect if IB is available (fetch ib0)
    if (m_view->yesNoQuestion(
            Messages::title, Messages::question, Messages::help)) {

        // @FIXME: Prompt for the version
        m_model->setOFED(cloyster::utils::enums::ofStringOpt<OFED::Kind>(
            m_view->listMenu(Messages::title, Messages::OFED::question,
                cloyster::utils::enums::toStrings<OFED::Kind>(), Messages::OFED::help))
                .value());
        LOG_DEBUG("Set OFED stack as: {}",
            cloyster::utils::enums::toString<OFED::Kind>(m_model->getOFED()->getKind()));

        try {
            Call<PresenterNetwork>(
                nc, Network::Profile::Application, Network::Type::Infiniband);
        } catch (const std::exception& ex) {
            LOG_ERROR("Failed to add {} network: {}",
                cloyster::utils::enums::toString(Network::Profile::Application),
                ex.what());
        }
    }
}

}
