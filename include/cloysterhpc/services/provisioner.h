/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_PROVISIONER_H_
#define CLOYSTERHPC_PROVISIONER_H_

#include <string>

// TODO: CFL Make this a generic interface between XCAT and Confluent
// try to use compile time polymorphims instead of runtime polymorphism
class Provisioner {
public:
    Provisioner(const Provisioner&) = delete;
    Provisioner(Provisioner&&) = delete;
    Provisioner& operator=(const Provisioner&) = delete;
    Provisioner& operator=(Provisioner&&) = delete;
    Provisioner() = default;
    virtual ~Provisioner() = default;

    virtual void install() = 0;
};

#endif // CLOYSTERHPC_PROVISIONER_H_
