/*
 * Copyright 2024 Arthur Mendes <arthurmco@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_DBUS_CLIENT_H_
#define CLOYSTERHPC_DBUS_CLIENT_H_

#include <any>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <type_traits>
#include <variant>

#include <cloysterhpc/messagebus.h>

class DBusClient : public MessageBus {

private:
    std::unique_ptr<sdbus::IProxy> m_proxy;

    //    std::unique_ptr<sdbus::IProxy>& getProxy();
    sdbus::MethodCall createMethodCall(
        std::string interface, std::string method);

public:
    DBusClient(std::string bus, std::string object)
        : m_proxy(
              sdbus::createProxy(std::move(sdbus::createSystemBusConnection()),
                  sdbus::ServiceName { bus },
                  sdbus::ObjectPath { object }))
    {
    }

    sdbus::MethodReply callMethod(const sdbus::MethodCall& message);

    std::unique_ptr<MessageBusMethod> method(
        std::string interface, std::string method);

    virtual ~DBusClient() { }
};

class DBusMethod : public MessageBusMethod {
    friend class DBusClient;

private:
    DBusClient* m_client;
    sdbus::MethodCall m_message;

    DBusMethod(DBusClient* c, sdbus::MethodCall&& message)
        : m_client(c)
        , m_message(message)
    {
    }

protected:
    virtual void pushSingleParam(MethodParamVariant param);
    virtual MessageReply callMethod();

public:
    DBusMethod() = delete;
};

#endif
