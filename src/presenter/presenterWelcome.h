//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_PRESENTERWELCOME_H
#define CLOYSTER_PRESENTERWELCOME_H

#include "presenterBase.h"
#include "../view/newtViewMessage.h"

class PresenterWelcome : public PresenterBase {
public:
    PresenterWelcome(std::unique_ptr<NewtViewMessage>,
                     std::unique_ptr<Cluster>);
    ~PresenterWelcome();
};

#endif //CLOYSTER_PRESENTERWELCOME_H
