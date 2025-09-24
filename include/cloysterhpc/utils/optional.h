#ifndef CLOYSTERHPC_UTILS_OPTIONAL_H_
#define CLOYSTERHPC_UTILS_OPTIONAL_H_

#include <fmt/core.h>
#include <optional>

namespace cloyster::utils::optional {

/**
 * @brief Unwrap the optional throwing std::runtime_error with the message if
 * opt has no value
 */
template <typename T, typename... Args>
T unwrap(
    std::optional<T> opt, fmt::format_string<Args...> format, Args&&... args)
{
    const auto msg = fmt::format(format, std::forward<decltype(args)>(args)...);
    if (!opt.has_value()) {
        throw std::runtime_error(msg);
    }
    return opt.value();
}

};

#endif
