/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/services/log.h>

#include <boost/algorithm/string.hpp>
#include <memory>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

template <typename Mutex>
class basic_restorable_file_sink final
    : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit basic_restorable_file_sink(
        const spdlog::filename_t& filename, bool truncate = false)
        : m_filename(filename)
    {
        m_file_helper.open(filename, truncate);
    }

    void suspendWrite()
    {
        m_is_file_opened = false;
        m_file_helper.close();
    }

    void restoreWrite()
    {
        m_file_helper.open(m_filename);

        for (const auto& buf : m_stored) {
            m_file_helper.write(buf);
        }
        m_file_helper.flush();

        m_is_file_opened = true;
        m_stored.clear();
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        if (m_is_file_opened) {
            m_file_helper.write(formatted);
        } else {
            m_stored.push_back(std::move(formatted));
        }
    }

    void flush_() override
    {
        if (m_is_file_opened) {
            m_file_helper.flush();
        }
    }

private:
    spdlog::filename_t m_filename;
    spdlog::details::file_helper m_file_helper;
    bool m_is_file_opened = true;
    std::vector<spdlog::memory_buf_t> m_stored;
};

using basic_restorable_file_sink_st
    = basic_restorable_file_sink<spdlog::details::null_mutex>;

namespace {
std::shared_ptr<basic_restorable_file_sink_st> fileSink;
}

void Log::init(std::size_t level) { init(static_cast<Level>(level)); }

void Log::init(Level level)
{
    const auto* const pattern { "%^[%Y-%m-%d %H:%M:%S.%e] %v%$" };

    auto stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderrSink->set_pattern(pattern);

    const auto& logfile = fmt::format(
        "{}.log", boost::to_lower_copy(std::string { productName }));

    fileSink = std::make_shared<basic_restorable_file_sink_st>(logfile);
    fileSink->set_pattern(pattern);

    std::vector<spdlog::sink_ptr> sinks { stderrSink, fileSink };
    auto logger = std::make_shared<spdlog::logger>(
        productName, sinks.begin(), sinks.end());

    logger->enable_backtrace(32);

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

void Log::suspendFileWrite() { fileSink->suspendWrite(); }

void Log::restoreFileWrite() { fileSink->restoreWrite(); }
