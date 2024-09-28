/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ISERVICE_H_
#define CLOYSTERHPC_ISERVICE_H_

#include <cloysterhpc/messagebus.h>

class IService {
private:
    std::shared_ptr<MessageBus> m_bus;
    std::string m_name;

    template <typename... Ts>
    sdbus::ObjectPath callObjectFunction(
        const std::string function, Ts... params)
    {
        MessageReply reply
            = m_bus->method("org.freedesktop.systemd1.Manager", function)
                  ->call(m_name, params...);
        return reply.get<sdbus::ObjectPath>();
    }

    template <typename... Ts>
    MessageReply callObjectFunctionArray(
        const std::string function, Ts... params)
    {
        std::vector<std::string> names = { m_name };
        MessageReply reply
            = m_bus->method("org.freedesktop.systemd1.Manager", function)
                  ->call(names, params...);
        return reply;
    }

public:
    IService(std::shared_ptr<MessageBus> bus, const std::string_view name)
        : m_bus(bus)
        , m_name(name)
    {
    }

    virtual ~IService() = default;

    void enable();
    void disable();
    void start();
    void restart();
    void stop();
};

#endif // CLOYSTERHPC_ISERVICE_H_
