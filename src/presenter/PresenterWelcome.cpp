//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterWelcome.h"

PresenterWelcome::PresenterWelcome(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    m_view->message(Messages::Welcome::message);
    LOG_DEBUG("Welcome message displayed");
}
