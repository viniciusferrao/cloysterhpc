/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ISERVICE_H_
#define CLOYSTERHPC_ISERVICE_H_

class IService {
protected:
    virtual void enable() = 0;
    [[maybe_unused]] virtual void disable() = 0;
    virtual void start() = 0;
    [[maybe_unused]] virtual void stop() = 0;

public:
    virtual ~IService() = default;
};

#endif // CLOYSTERHPC_ISERVICE_H_
