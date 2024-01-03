/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERHOSTID_H_
#define CLOYSTERHPC_PRESENTERHOSTID_H_

#include <cloysterhpc/presenter/Presenter.h>

class PresenterHostId : public Presenter {
private:
    struct Messages {
        static constexpr const auto title = "Hostname settings";
        static constexpr const auto question
            = "Enter the desired hostname and domain name for this machine";
        static constexpr const auto help
            = Presenter::Messages::Placeholder::help;

        static constexpr const auto hostname = "Hostname";
        static constexpr const auto domainName = "Domain name";
    };

public:
    PresenterHostId(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERHOSTID_H_
