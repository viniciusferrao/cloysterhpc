#pragma once

#include <any>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <type_traits>
#include <variant>

/**
 * Not much to be done here... just our simple needs for an answer type
 *
 * We also require compatibility with sdbus, but sdbus errors are enough to
 * tell this to the user
 */
template <typename T>
concept MessageReturnable = std::is_nothrow_default_constructible_v<
    T> && std::is_nothrow_move_constructible_v<T>;

/**
 * A more or less "generic"-ish way to refer to a message reply
 * Encapsulates the sdbus answer so we can mock answers for testing
 *
 * Unfortunately, to increase testability (and mockability), we need
 * to make this class aware of both the real answer type and the mock
 * answer type
 *
 * This will be worth when we start mocking services, I promise...
 */
class MessageReply {
private:
    std::variant<sdbus::MethodReply, std::any> m_base_reply;

public:
    explicit MessageReply(sdbus::MethodReply&& reply)
        : m_base_reply(reply)
    {
    }

    explicit MessageReply(std::any&& reply)
        : m_base_reply(reply)
    {
    }

    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template <MessageReturnable T> T get()
    {
        return std::visit(
            overloaded {
                [](sdbus::MethodReply arg) {
                    T result {};
                    arg >> result;
                    return result;
                },
                [](std::any arg) { return std::any_cast<T>(arg); },
            },
            m_base_reply);
    }

    /**
     * If your DBus function returns two values, use this method
     *
     * This seems to be rare, so making this function generic to accept
     * a bigger tuple might seem a lot of trouble for minimum gain
     */
    template <MessageReturnable T1, MessageReturnable T2>
    std::tuple<T1, T2> getPair()
    {
        return std::visit(overloaded {
                              [](sdbus::MethodReply arg) {
                                  T1 result1 {};
                                  T2 result2 {};
                                  arg >> result1;
                                  arg >> result2;
                                  return std::make_tuple<T1, T2>(
                                      std::move(result1), std::move(result2));
                              },
                              [](std::any arg) {
                                  return std::any_cast<std::tuple<T1, T2>>(arg);
                              },
                          },
            m_base_reply);
    }
};

/**
 * All possible parameter types for dbus (to be filled)
 *
 * For more complex types (rarely from what I saw) please add them
 * here or use sdbus-cpp directly... it is not hard, but you will
 * lose mockability.
 */
using MethodParamVariant
    = std::variant<unsigned char, short, int, long int, bool, std::string,
        const char*, sdbus::ObjectPath, std::vector<unsigned char>,
        std::vector<std::string>, std::vector<sdbus::ObjectPath>>;

class MessageBusMethod {
private:
    bool finished = false;

protected:
    virtual void pushSingleParam(MethodParamVariant param) = 0;
    virtual MessageReply callMethod() = 0;

public:
    void addParams()
    {
        // end case for the variadic template below
    }

    template <typename T, typename... Ts> void addParams(T param, Ts... params)
    {
        this->pushSingleParam(param);
        this->addParams(params...);
    }

    template <typename... Ts> MessageReply call(Ts... params)
    {
        if (finished) {
            throw std::runtime_error {
                "Only one ->call() is permitted per method()"
            };
        }

        this->addParams(params...);

        finished = true;
        return this->callMethod();
    }

    virtual ~MessageBusMethod() = default;
};

/**
 * A class for communicating with a message bus (usually dbus, but this can be
 * changed with some effort.
 *
 * You need to instantiate some specific bus (like DBusClient or the
 * TestBusClient) and call a function there
 *
 * The syntax is like this:
 * ```cpp
 *
 * std::unique_ptr<MessageBus> client = make_a_bus_in_some_way();
 * auto replyObject = client->method("org.mybus.MyInterface",
 * "MyMethod")->call(param1, param2); auto reply =
 * replyObject.get<MyMethodReturnType>();
 * ```
 *
 * Check sdbus guide for the method return types, we will copy theirs because it
 * is not worth to encapsulate (too hard with no gain)
 */
class MessageBus {
public:
    [[nodiscard]] virtual std::unique_ptr<MessageBusMethod> method(
        std::string interface, std::string method)
        = 0;
    virtual ~MessageBus() = default;
};
