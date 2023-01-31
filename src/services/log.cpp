/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "log.h"

#include <boost/algorithm/string.hpp>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void Log::init(std::size_t level) { init(static_cast<Level>(level)); }

void Log::init(Level level)
{
    const auto& pattern { "%^[%Y-%m-%d %H:%M:%S.%e] %v%$" };

    auto stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderrSink->set_pattern(pattern);

    const auto& logfile = fmt::format(
        "{}.log", boost::to_lower_copy(std::string { productName }));

    auto fileSink
        = std::make_shared<spdlog::sinks::basic_file_sink_st>(logfile);
    fileSink->set_pattern(pattern);

    std::vector<spdlog::sink_ptr> sinks { stderrSink, fileSink };
    auto logger = std::make_shared<spdlog::logger>(
        productName, sinks.begin(), sinks.end());

    switch (level) {
        using enum Log::Level;

        case Off:
            logger->set_level(spdlog::level::off);
            logger->flush_on(spdlog::level::off);
            break;
        case Critical:
            logger->set_level(spdlog::level::critical);
            logger->flush_on(spdlog::level::critical);
            break;
        case Error:
            logger->set_level(spdlog::level::err);
            logger->flush_on(spdlog::level::err);
            break;
        case Warn:
            logger->set_level(spdlog::level::warn);
            logger->flush_on(spdlog::level::warn);
            break;
        case Info:
            logger->set_level(spdlog::level::info);
            logger->flush_on(spdlog::level::info);
            break;
        case Debug:
            logger->set_level(spdlog::level::debug);
            logger->flush_on(spdlog::level::debug);
            break;
        case Trace:
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);
            break;
    }

    spdlog::register_logger(logger);
}

void Log::shutdown() { spdlog::shutdown(); }
