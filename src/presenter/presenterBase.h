//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_PRESENTERBASE_H
#define CLOYSTER_PRESENTERBASE_H

#include "../cluster.h"
#include "../view/newtViewBase.h"

class PresenterBase {
public:
    PresenterBase(std::unique_ptr<NewtViewBase> view,
                  std::unique_ptr<Cluster> model);
    ~PresenterBase();

    //void notifyEvent() override;

    /* TODO: Move to top to better organize */
private:
    std::unique_ptr<Cluster> m_model;
    std::unique_ptr<NewtViewBase> m_view;

    void welcome();
};

#endif //CLOYSTER_PRESENTERBASE_H
