/**
 * @file cloyster_utils_enum.h
 * @brief Utility functions for enum manipulation using the magic_enum library.
 *
 * This header provides a set of utility functions within the
 * `cloyster::utils::enums` namespace to simplify working with C++ enums. It
 * wraps the `magic_enum` library to offer features such as converting enums to
 * strings, retrieving all enum names, counting enum values, and parsing strings
 * back to enums with optional case-insensitive matching.
 *
 * @note This header depends on the `magic_enum` library
 *       (https://github.com/Neargye/magic_enum) for enum reflection
 * capabilities.
 *
 */

#ifndef CLOYSTER_UTILS_ENUM_H
#define CLOYSTER_UTILS_ENUM_H

#include <cstdint>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <type_traits>
#include <vector>

namespace cloyster::utils::enums {

/**
 * @enum Case
 * @brief Specifies case sensitivity for string-to-enum conversions.
 *
 * This enum defines options for controlling whether string parsing should be
 * case-sensitive or case-insensitive when converting strings to enum values.
 */
enum class Case : std::uint8_t {
    Sensitive, ///< Case-sensitive matching (exact match required).
    Insensitive ///< Case-insensitive matching (e.g., "Value" matches "value").
};

/**
 * @brief Converts an enum value to its string representation.
 * @tparam T The enum type to convert. Must be an enumeration type.
 * @param enumValue The enum value to convert.
 * @return The string name of the enum value, or an empty string if invalid.
 */
template <typename T>
    requires std::is_enum_v<T>
std::string toString(T enumValue)
{
    return static_cast<std::string>(magic_enum::enum_name<T>(enumValue));
}

/**
 * @brief Retrieves all valid names of an enum type as a vector of strings.
 * @tparam T The enum type to query. Must be an enumeration type.
 * @return A vector containing the string names of all enum values.
 */
template <typename T> std::vector<std::string> toStrings()
{
    auto array = magic_enum::enum_names<T>();
    std::vector<std::string> output(array.begin(), array.end());
    return output;
}

/**
 * @brief Returns the number of values in an enum type.
 * @tparam T The enum type to count. Must be an enumeration type.
 * @return The total number of enum values.
 */
template <typename T> constexpr std::size_t count()
{
    return magic_enum::enum_count<T>();
}

/**
 * @brief Parses a string to an enum value with configurable case sensitivity.
 * @tparam T The enum type to parse into. Must be an enumeration type.
 * @param str The string to parse (e.g., "Value", "value").
 * @param case_ The case sensitivity mode (default: Case::Sensitive).
 * @return An optional containing the parsed enum value, or std::nullopt if the
 * string does not match any enum value.
 */
template <typename T>
    requires std::is_enum_v<T>
std::optional<T> ofStringOpt(std::string_view str, Case case_ = Case::Sensitive)
{
    if (case_ == Case::Insensitive) {
        return magic_enum::enum_cast<T>(str, magic_enum::case_insensitive);
    }

    return magic_enum::enum_cast<T>(str);
}

/**
 * @brief Example usage of the enum utility functions.
 * @code
 * enum class Color { RED, GREEN, BLUE };
 * std::string name = cloyster::utils::enums::toString(Color::GREEN); // "GREEN"
 * auto colors = cloyster::utils::enums::toStrings<Color>(); // {"RED", "GREEN",
 * "BLUE"} std::size_t count = cloyster::utils::enums::count<Color>(); // 3 auto
 * parsed = cloyster::utils::enums::ofStringOpt<Color>("blue",
 * Case::Insensitive); if (parsed) { std::cout <<
 * cloyster::utils::enums::toString(*parsed) << "\n"; } // "BLUE"
 * @endcode
 */

} // namespace cloyster::utils::enums

#endif // CLOYSTER_UTILS_ENUM_H
