//
// Created by Vinícius Ferrão on 23/11/21.
//

#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

void Log::init(Level level) {
    auto stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderrSink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] %v%$");

    std::vector<spdlog::sink_ptr> sinks{stderrSink};
    auto logger = std::make_shared<spdlog::logger>(
            productName, sinks.begin(), sinks.end());

    switch (level) {
        case Log::Level::Trace:
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);
            break;
        case Log::Level::Debug:
            logger->set_level(spdlog::level::debug);
            logger->flush_on(spdlog::level::debug);
            break;
        case Log::Level::Info:
            logger->set_level(spdlog::level::info);
            logger->flush_on(spdlog::level::info);
            break;
        case Log::Level::Warn:
            logger->set_level(spdlog::level::warn);
            logger->flush_on(spdlog::level::warn);
            break;
        case Log::Level::Error:
            logger->set_level(spdlog::level::err);
            logger->flush_on(spdlog::level::err);
            break;
        case Log::Level::Critical:
            logger->set_level(spdlog::level::critical);
            logger->flush_on(spdlog::level::critical);
            break;
        case Log::Level::Off:
            logger->set_level(spdlog::level::off);
            logger->flush_on(spdlog::level::off);
            break;
    }

    spdlog::register_logger(logger);
}

void Log::shutdown() {
    spdlog::shutdown();
}
