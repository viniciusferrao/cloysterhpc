/*
 * Copyright 2023 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_ISERVICE_H_
#define CLOYSTERHPC_ISERVICE_H_

#include <cloysterhpc/messagebus.h>
#include <cloysterhpc/patterns/singleton.h>
#include <stdexcept>
#include <string_view>

namespace cloyster::services {

class daemon_exception : public std::runtime_error {
public:
    explicit daemon_exception(const char* msg)
        : std::runtime_error(msg)
    {
    }

    explicit daemon_exception(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
};

class IService {
private:
    std::string m_name;

    bool handleException(const sdbus::Error& e, const std::string_view fn);

    template <typename... Ts>
    std::optional<sdbus::ObjectPath> callObjectFunction(
        const std::string& function, Ts... params)
    {
        auto bus = cloyster::Singleton<cloyster::services::MessageBus>::get();
        try {
            MessageReply reply
                = bus->method("org.freedesktop.systemd1.Manager", function)
                      ->call(m_name, params...);
            return reply.get<sdbus::ObjectPath>();
        } catch (sdbus::Error& e) {
            if (!handleException(e, function)) {
                throw;
            }
            return {};
        }
    }

    template <typename... Ts>
    std::optional<MessageReply> callObjectFunctionArray(
        const std::string& function, Ts... params)
    {
        auto bus = cloyster::Singleton<cloyster::services::MessageBus>::get();
        try {
            std::vector<std::string> names = { m_name };
            MessageReply reply
                = bus->method("org.freedesktop.systemd1.Manager", function)
                      ->call(names, params...);
            return reply;
        } catch (sdbus::Error& e) {
            if (!handleException(e, function)) {
                throw;
            }
            return {};
        }
    }

public:
    IService(const IService&) = default;
    IService(IService&&) = delete;
    IService& operator=(const IService&) = default;
    IService& operator=(IService&&) = delete;
    explicit IService(const std::string_view name)
        : m_name(name)
    {
    }

    virtual ~IService() = default;

    void enable();
    void disable();
    void start();
    void restart();
    void stop();
};

}; // namespace cloyster::services

#endif // CLOYSTERHPC_ISERVICE_H_
