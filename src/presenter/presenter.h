//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTER_H
#define CLOYSTERHPC_PRESENTER_H

#include <memory>
#include "../cluster.h"
#include "../view/newt.h"

class Presenter {
protected:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    Presenter(std::unique_ptr<Cluster>& model,
              std::unique_ptr<Newt>& view)
              : m_model(model)
              , m_view(view) {}
};

#endif //CLOYSTERHPC_PRESENTER_H
