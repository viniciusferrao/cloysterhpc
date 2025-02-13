/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTER_H_
#define CLOYSTERHPC_PRESENTER_H_

#include <array>
#include <cloysterhpc/cluster.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/view/newt.h>
#include <memory>
#include <string>
#include <vector>

class Presenter {
protected:
    // FIXME: According to Herb Sutter we shouldn't be passing this as const
    // ref&
    //  - https://youtu.be/xnqTKD8uD64?t=1145
    //  - http://tinyurl.com/gotw91
    std::unique_ptr<Cluster>& m_model;
    std::unique_ptr<Newt>& m_view;

    struct Messages {
        struct Placeholder {
            static constexpr const auto help
                = "We're sorry, but there's no help available for this session";
        };
    };

    Presenter(std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
        : m_model(model)
        , m_view(view)
    {
    }

    template <class C, typename... Args> void Call(Args&&... args)
    {
        C(m_model, m_view, std::forward<Args>(args)...);
    }
};

#endif // CLOYSTERHPC_PRESENTER_H_
