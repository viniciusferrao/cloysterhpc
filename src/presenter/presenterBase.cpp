//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "presenterBase.h"
#include "presenterWelcome.h"

PresenterBase::PresenterBase (std::unique_ptr<NewtViewBase> view,
                              std::unique_ptr<Cluster> model)
                              : m_model(std::move(model)),
                                m_view(std::move(view)) {

    //m_view->subscribe(this);
#if 0
    auto welcome = std::make_unique<WelcomePresenter>(&m_view,
                                                      &m_model);
#endif
    //m_view->welcomeMessage();

#if 0
    m_view->timezoneSelection({"Teste1", "Teste2"});
    m_view->localeSelection({ "en.US_UTF-8", "pt.BR_UTF-8", "C" });
#endif

}

PresenterBase::~PresenterBase() {

}

#if 0
void PresenterBase::notifyEvent () {
    std::vector<std::string> name = m_view->getAnswers();
    std::string text = m_model->generateText(name[0]);
    m_view->setLabelText(text);
}
#endif