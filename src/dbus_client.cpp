#include <cloysterhpc/dbus_client.h>

// DBusClient

sdbus::MethodCall DBusClient::createMethodCall(
    std::string interface, std::string method)
{
    sdbus::InterfaceName interfaceInst { interface };
    sdbus::MethodName methodInst { method };
    return this->m_proxy->createMethodCall(interfaceInst, methodInst);
}

sdbus::MethodReply DBusClient::callMethod(const sdbus::MethodCall& method)
{
    return this->m_proxy->callMethod(method);
}

std::unique_ptr<MessageBusMethod> DBusClient::method(
    std::string interface, std::string method)
{
    auto m = this->createMethodCall(interface, method);
    std::unique_ptr<MessageBusMethod> mtd(
        new DBusMethod { this, std::move(m) });
    return mtd;
}

// DBusMethod

void DBusMethod::pushSingleParam(MethodParamVariant param)
{
    std::visit([this](auto&& arg) { this->m_message << arg; }, param);
}

MessageReply DBusMethod::callMethod()
{
    return MessageReply { std::move(m_client->callMethod(this->m_message)) };
}
