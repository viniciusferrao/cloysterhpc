//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterInfiniband.h"

PresenterInfiniband::PresenterInfiniband(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    // TODO: Infiniband class? Detect if IB is available (fetch ib0)
    if (m_view->yesNoQuestion(Messages::title, Messages::question, Messages::help)) {

        m_model->setOFED(magic_enum::enum_cast<Cluster::OFED>(
                m_view->listMenu(Messages::title,
                                 Messages::OFED::question,
                                 magic_enum::enum_names<Cluster::OFED>(),
                                 Messages::OFED::help)).value()
        );
        LOG_INFO("Set OFED stack as: {}", magic_enum::enum_name<Cluster::OFED>(m_model->getOFED()));

        try {
            Call<PresenterNetwork>(Network::Profile::Application, Network::Type::Infiniband);
        } catch (const std::exception& ex) {
            LOG_WARN("Failed to add {} network: {}",
                     magic_enum::enum_name(Network::Profile::Application),
                     ex.what());
        }
    }
}
