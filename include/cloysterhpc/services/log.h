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
/**
 * @brief Logs a critical message.
 *
 * This macro logs a critical level message if the logger is initialized.
 *
 * @param __VA_ARGS__ The message to log and its format arguments.
 */
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
#define LOG_DEBUG(...)                                                         \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->debug(__VA_ARGS__);                          \
    }

// Available only with DEBUG builds
#ifndef NDEBUG
#define LOG_TRACE(...)                                                         \
    if (spdlog::get(productName) != nullptr) {                                 \
        spdlog::get(productName)->trace(__VA_ARGS__);                          \
    }

/**
 * @brief Asserts a condition and logs a critical message if the assertion
 * fails.
 *
 * This macro asserts a condition and logs a critical message if the condition
 * is false. Only available in debug builds.
 *
 * @param x The condition to assert.
 * @param msg The message to log if the assertion fails.
 */
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
#define LOG_ASSERT(x, msg) (void)0;
#endif

/**
 * @namespace Log
 * @brief Provides logging functionality.
 *
 * This namespace contains functions to initialize and manage the logging
 * system.
 */
namespace Log {
/**
 * @enum Level
 * @brief Defines the logging levels.
 *
 * This enum defines the various levels of logging available.
 */
enum class Level { Off, Critical, Error, Warn, Info, Debug, Trace };

/**
 * @brief Initializes the logging system with a specified logging level.
 *
 * @param level The logging level as a size_t.
 */
void init(std::size_t level);
/**
 * @brief Initializes the logging system with a specified logging level.
 *
 * @param level The logging level as a Level enum. Default is Level::Info.
 */
void init(Level level = Level::Info);
/**
 * @brief Shuts down the logging system.
 *
 * This function cleans up and shuts down the logging system.
 */
void shutdown();
}

#endif // CLOYSTERHPC_LOG_H_
