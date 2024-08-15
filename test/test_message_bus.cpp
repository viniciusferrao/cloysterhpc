#include <cstdio>
#include <testing/test_message_bus.h>

unsigned TestMessageBus::callCount(FunctionStore s) const
{
    auto sname = makeStoreName(s);
    if (!m_functions.contains(sname)) {
        return 0;
    } else {
        return m_functions.at(sname).size();
    }
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
    auto reply = std::make_any<sdbus::ObjectPath>(sdbus::ObjectPath("123"));
    return MessageReply(std::move(reply));
}
