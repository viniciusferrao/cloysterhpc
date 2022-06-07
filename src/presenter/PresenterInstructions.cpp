//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterInstructions.h"

PresenterInstructions::PresenterInstructions(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    m_view->message(Messages::Instructions::message);
    LOG_DEBUG("Install instructions displayed");
}
