#include <cstdio>
#include <optional>
#include <testing/test_message_bus.h>

unsigned TestMessageBus::callCount(FunctionStore s) const
{
    auto sname = makeStoreName(s);
    if (!m_functions.contains(sname)) {
        return 0;
    } else {
        return unsigned(m_functions.at(sname).size());
    }
}

void TestMessageBus::registerResponse(FunctionStore s, std::any&& response)
{
    auto sname = makeStoreName(s);
    m_results[sname] = std::move(response);
}

std::optional<std::any> TestMessageBus::returnResponse(FunctionStore s)
{
    auto sname = makeStoreName(s);
    if (auto search = m_results.find(sname); search != m_results.end())
        return std::make_optional(search->second);
    else
        return std::nullopt;
}

FunctionParameters TestMessageBus::calledWith(
    FunctionStore s, unsigned index) const
{
    if (this->callCount(s) == 0 || index >= this->callCount(s)) {
        return {};
    }

    auto sname = makeStoreName(s);
    return m_functions.at(sname).at(index);
}

void TestMessageBus::dump() const
{
    fprintf(stderr, "???\n");
    for (const auto& [k, paramslist] : m_functions) {
        fprintf(stderr, "\t%s: %ld calls\n", k.c_str(), paramslist.size());
        int i = 0;
        for (const auto& params : paramslist) {
            fprintf(stderr, "\t\t%d: ", i++);
            for (const auto& p : params) {
                fprintf(stderr, "<%s> ", p.type().name());
            }
            fprintf(stderr, "\n");
        }
    }
}

std::unique_ptr<MessageBusMethod> TestMessageBus::method(
    std::string interface, std::string method)
{
    return std::unique_ptr<MessageBusMethod>(
        new TestMessageMethod { this, std::make_tuple(interface, method) });
}

MessageReply TestMessageMethod::callMethod()
{
    m_bus->registerCall(m_store, m_params);
    auto reply = m_bus->returnResponse(m_store).value_or(
        std::make_any<sdbus::ObjectPath>(sdbus::ObjectPath("123")));
    return MessageReply(std::move(reply));
}
