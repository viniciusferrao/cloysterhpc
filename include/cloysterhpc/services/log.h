/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_LOG_H_
#define CLOYSTERHPC_LOG_H_

#include <cloysterhpc/const.h>
#include <spdlog/spdlog.h>

// Define breakpoints per OS in case of custom assertion failures
#if __APPLE__
#define LOG_BREAK __builtin_debugtrap()
#else // Linux or others Unixes
#define LOG_BREAK __builtin_trap()
#endif

// Define some macros to ease the logging process
#define LOG_CRITICAL(...)                                                      \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->critical(__VA_ARGS__);                       \
    }
#define LOG_ERROR(...)                                                         \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->error(__VA_ARGS__);                          \
    }
#define LOG_WARN(...)                                                          \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->warn(__VA_ARGS__);                           \
    }
#define LOG_INFO(...)                                                          \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->info(__VA_ARGS__);                           \
    }

// Available only with DEBUG builds
#ifndef NDEBUG
#define LOG_DEBUG(...)                                                         \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->debug(__VA_ARGS__);                          \
    }
#define LOG_TRACE(...)                                                         \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->trace(__VA_ARGS__);                          \
    }

#define LOG_ASSERT(x, msg)                                                     \
    if ((x)) {                                                                 \
    } else {                                                                   \
        LOG_CRITICAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x,    \
            msg, __FILE__, __LINE__);                                          \
        LOG_BREAK;                                                             \
    }
#else
// Disabled for RELEASE builds
#define LOG_TRACE(...) (void)0;
#define LOG_DEBUG(...) (void)0;
#define LOG_ASSERT(x, msg) (void)0;
#endif

namespace Log {
enum class Level { Off, Critical, Error, Warn, Info, Debug, Trace };

void init(std::size_t level);
void init(Level level = Level::Info);
void shutdown();
}

#endif // CLOYSTERHPC_LOG_H_
