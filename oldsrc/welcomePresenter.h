//
// Created by Vinícius Ferrão on 30/10/21.
//
#if 0
#ifndef CLOYSTER_WELCOMEPRESENTER_H
#define CLOYSTER_WELCOMEPRESENTER_H

#include "../viewSubscriber.h"
#include "../viewTerminalUI.h"
#include "../cluster.h"

class WelcomePresenter : public viewSubscriber {
public:
    WelcomePresenter(std::shared_ptr<viewTerminalUI> view,
                     std::shared_ptr<Cluster> model);
    ~WelcomePresenter();

    void notifyEvent() override;

    /* TODO: Move to top to better organize */
private:
    std::shared_ptr<Cluster> m_model;
    std::shared_ptr<viewTerminalUI> m_view;
};

#endif //CLOYSTER_WELCOMEPRESENTER_H
#endif
