/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PresenterInfiniband.h"

#include <algorithm>

PresenterInfiniband::PresenterInfiniband(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{

    auto interfaces = Connection::fetchInterfaces();
    if (std::find(interfaces.begin(), interfaces.end(), "ib0")
        == interfaces.end()) {
        LOG_WARN("No Infiniband interfaces found.");
        return;
    }

    // TODO: Infiniband class? Detect if IB is available (fetch ib0)
    if (m_view->yesNoQuestion(
            Messages::title, Messages::question, Messages::help)) {

        m_model->setOFED(magic_enum::enum_cast<OFED::Kind>(
            m_view->listMenu(Messages::title, Messages::OFED::question,
                magic_enum::enum_names<OFED::Kind>(), Messages::OFED::help))
                             .value());
        LOG_DEBUG("Set OFED stack as: {}",
            magic_enum::enum_name<OFED::Kind>(m_model->getOFED()->getKind()));

        try {
            Call<PresenterNetwork>(
                Network::Profile::Application, Network::Type::Infiniband);
        } catch (const std::exception& ex) {
            LOG_ERROR("Failed to add {} network: {}",
                magic_enum::enum_name(Network::Profile::Application),
                ex.what());
        }
    }
}
