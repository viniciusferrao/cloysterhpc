#pragma once

#include <memory>
#include <string>

#include <cloysterhpc/messagebus.h>

class DaemonHandler {
private:
    std::shared_ptr<MessageBus> m_bus;
    const std::string m_name;

    template <typename... Ts>
    sdbus::ObjectPath callObjectFunction(
        const std::string function, Ts... params)
    {
        MessageReply reply
            = m_bus->method("org.freedesktop.systemd1.Manager", function)
                  ->call(m_name, params...);
        return reply.get<sdbus::ObjectPath>();
    }

    static std::string fixServiceName(std::string name);

public:
    DaemonHandler(std::shared_ptr<MessageBus> bus, const std::string& name)
        : m_bus(bus)
        , m_name(DaemonHandler::fixServiceName(name))
    {
    }

    static void daemonReload(MessageBus& bus);

    bool exists();

    void load();
    void start();
    void stop();
    void restart();
    void enable();
    void disable();
};
