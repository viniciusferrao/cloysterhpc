/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PRESENTERHOSTID_H_
#define CLOYSTERHPC_PRESENTERHOSTID_H_

#include "Presenter.h"

class PresenterHostId : public Presenter {
private:
    struct Messages {
        static constexpr const char* title = "Hostname settings";
        static constexpr const char* question
            = "Enter the desired hostname and domain name for this machine";
        static constexpr const char* help
            = Presenter::Messages::Placeholder::help;

        static constexpr const char* hostname = "Hostname";
        static constexpr const char* domainName = "Domain name";
    };

public:
    PresenterHostId(
        std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view);
};

#endif // CLOYSTERHPC_PRESENTERHOSTID_H_
