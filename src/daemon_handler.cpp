#include <cloysterhpc/daemon_handler.h>

std::string DaemonHandler::fixServiceName(std::string name)
{
    if (!name.ends_with(".service")) {
        return name + ".service";
    } else {
        return name;
    }
}

bool DaemonHandler::exists()
{
    auto path = this->callObjectFunction("GetUnit");
    return !path.empty();
}

void DaemonHandler::start()
{
    this->callObjectFunction("StartUnit", "replace");
}

void DaemonHandler::stop() { this->callObjectFunction("StopUnit", "replace"); }

void DaemonHandler::load() { this->callObjectFunction("LoadUnit"); }

void DaemonHandler::restart()
{
    this->callObjectFunction("RestartUnit", "replace");
}

void DaemonHandler::daemonReload(MessageBus& bus)
{
    bus.method("org.freedesktop.systemd1.Manager", "Reload")->call();
}

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

#include <testing/test_message_bus.h>

/*
TEST_SUITE("Test service handler connectivity")
{
    TEST_CASE("Service handler testing")
    {
        auto testBus = std::make_shared<TestMessageBus>();

        DaemonHandler sr { testBus, "generic-service" };

        SUBCASE("it can start")
        {
            auto function = std::make_tuple<std::string, std::string>(
                "org.freedesktop.systemd1.Manager", "StartUnit");

            REQUIRE(testBus->callCount(function) == 0);
            sr.start();

            REQUIRE(testBus->callCount(function) == 1);
            auto params = testBus->calledWith(function, 0);
            CHECK(std::any_cast<std::string>(params[0])
                == "generic-service.service");
        }

        SUBCASE("it can stop")
        {
            auto function = std::make_tuple<std::string, std::string>(
                "org.freedesktop.systemd1.Manager", "StopUnit");

            REQUIRE(testBus->callCount(function) == 0);
            sr.stop();

            REQUIRE(testBus->callCount(function) == 1);
            auto params = testBus->calledWith(function, 0);
            CHECK(std::any_cast<std::string>(params[0])
                == "generic-service.service");
        }

        SUBCASE("it can restart")
        {
            auto function = std::make_tuple<std::string, std::string>(
                "org.freedesktop.systemd1.Manager", "RestartUnit");

            REQUIRE(testBus->callCount(function) == 0);
            sr.restart();

            REQUIRE(testBus->callCount(function) == 1);
            auto params = testBus->calledWith(function, 0);
            CHECK(std::any_cast<std::string>(params[0])
                == "generic-service.service");
        }
    }
}
*/
