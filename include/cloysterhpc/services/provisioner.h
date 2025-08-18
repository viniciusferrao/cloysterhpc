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
    virtual ~Provisioner() = default;

    // virtual void method () = 0;
};

#endif // CLOYSTERHPC_PROVISIONER_H_
