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
    // FIXME: According to Herb Sutter we shouldn't be passing this as const ref&
    //  - https://youtu.be/xnqTKD8uD64?t=1145
    //  - http://tinyurl.com/gotw91
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

    template<class C, typename... Args>
    void Call(Args&&... args)
    {
        C(m_model, m_view, std::forward<Args>(args)...);
    }
};

#endif //CLOYSTERHPC_PRESENTER_H
