//
// Created by Vinícius Ferrão on 02/03/22.
//

#ifndef CLOYSTERHPC_PRESENTER_H
#define CLOYSTERHPC_PRESENTER_H

#include <memory>
#include <array>
#include <vector>
#include <string>
#include "../services/log.h"
#include "../cluster.h"
#include "../view/newt.h"

class Presenter {
protected:
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    struct Messages {
        struct Placeholder {
            static constexpr const char* help = "We're sorry, but there's no help available for this session";
        };
    };

    Presenter(std::unique_ptr<Cluster>& model,
              std::unique_ptr<Newt>& view)
              : m_model(model)
              , m_view(view)
    {}

    template<class C, typename... T>
    void Call(T&&... t)
    {
        C(m_model, m_view, t...);
    }
};

#endif //CLOYSTERHPC_PRESENTER_H
