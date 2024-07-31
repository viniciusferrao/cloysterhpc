#pragma once

#include <any>
#include <cloysterhpc/messagebus.h>
#include <string>
#include <tuple>
#include <vector>

class TestMessageMethod;

using FunctionStore = std::tuple<std::string, std::string>;
using FunctionParameters = std::vector<std::any>;

class TestMessageBus : public MessageBus {
    friend class TestMessageMethod;

private:
    std::unordered_map<std::string, std::vector<FunctionParameters>>
        m_functions;

    std::string makeStoreName(FunctionStore s) const
    {
        return std::get<0>(s) + "->" + std::get<1>(s);
    }

    void registerCall(FunctionStore s, FunctionParameters p)
    {
        auto count = this->callCount(s);
        m_functions[makeStoreName(s)].push_back(p);
    }

public:
    std::unique_ptr<MessageBusMethod> method(
        std::string interface, std::string method);

    void dump() const;

    unsigned callCount(FunctionStore s) const;
    FunctionParameters calledWith(FunctionStore s, unsigned index) const;
};

class TestMessageMethod : public MessageBusMethod {
private:
    TestMessageBus* m_bus;
    FunctionStore m_store;
    FunctionParameters m_params = {};

protected:
    virtual void pushSingleParam(MethodParamVariant param)
    {
        std::visit(
            [this](auto&& arg) { this->m_params.push_back(arg); }, param);
    }
    virtual MessageReply callMethod();

public:
    TestMessageMethod(TestMessageBus* bus, FunctionStore s)
        : m_bus(bus)
        , m_store(s)
    {
    }
};
