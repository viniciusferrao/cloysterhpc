#ifndef CLOYSTERHPC_UTILS_CONTAINER
#define CLOYSTERHPC_UTILS_CONTAINER

#include <boost/asio/ip/address.hpp>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/utils/optional.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::utils::containers::associative {

/**
 * @concept AssociativeContainer
 * @brief A C++20 concept to check if a type is a key-value associative
 * container.
 *
 * This concept verifies that a type `C` meets the requirements to be used
 * with key-based lookup functions. It ensures the container has a `mapped_type`
 * and a `find()` member function that takes a key and returns a
 * `const_iterator`. This allows for compile-time checking and function
 * overloading based on container properties.
 *
 * It is expected to work with std::map and std::unordered_map
 *
 * @tparam C The type of the container.
 * @tparam K The type of the key to be searched.
 */
template <typename C, typename K>
concept AssociativeContainer = requires(const C& c, const K& key) {
    typename C::mapped_type;
    { c.find(key) } -> std::same_as<typename C::const_iterator>;
};

/**
 * @brief Finds a value by key in an associative container and returns it as an
 * optional.
 *
 * This function provides a safe and idiomatic way to search for an element by
 * key. If the key is found, it returns a `std::optional` containing the
 * corresponding value. If the key is not found, it returns a `std::nullopt`.
 * This prevents the need for manual checks against `container.end()` and avoids
 * returning default-constructed values.
 *
 * @tparam Container The type of the associative container.
 * @tparam Key The type of the key.
 * @param container The container to search within.
 * @param key The key to look for.
 * @return `std::optional<typename Container::mapped_type>` containing the value
 * if found, or `std::nullopt` otherwise.
 */
template <typename Container, typename Key>
    requires AssociativeContainer<Container, Key>
std::optional<typename Container::mapped_type> findOpt(
    const Container& container, const Key& key) noexcept
{
    auto iter = container.find(key);

    if (iter != container.end()) {
        return iter->second;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Finds a value by key in an associative container, throwing an
 * exception if not found.
 *
 * This function is a convenience wrapper around `findOpt`. It provides
 * "fail-fast" behavior by immediately throwing a formatted exception if the key
 * is not present in the container. This is useful for cases where the key is
 * expected to exist and a failure to find it indicates a programming error.
 *
 * The exception message is generated using a `fmt::format_string`, allowing for
 * detailed, user-defined error messages.
 *
 * @tparam Container The type of the associative container.
 * @tparam Key The type of the key.
 * @tparam Args The types of the arguments for the format string.
 * @param container The container to search within.
 * @param key The key to look for.
 * @param fmt A format string for the error message if the key is not found.
 * @param args The arguments for the format string.
 * @return The value associated with the key.
 * @throws std::runtime_error if the key is not found.
 */
template <typename Container, typename Key, typename... Args>
    requires AssociativeContainer<Container, Key>
typename Container::mapped_type findExc(const Container& container,
    const Key& key, fmt::format_string<Args...> fmt, Args&&... args)
{
    return optional::unwrap(
        findOpt(container, key), fmt, std::forward<Args>(args)...);
}

TEST_SUITE_BEGIN("cloyster::utils::containers::associative");

TEST_CASE("findOpt")
{
    std::unordered_map<int, int> map = {
        { 0, 0 },
        { 1, 1 },
        { 2, 2 },
    };

    CHECK(findOpt(map, 0).value() == 0);
    CHECK(findOpt(map, 1).value() == 1);
    CHECK(findOpt(map, 2).value() == 2);
    CHECK(findExc(map, 0, "Some error msg") == 0);
    CHECK(findExc(map, 1, "Some error msg") == 1);
    CHECK(findExc(map, 2, "Some error msg") == 2);

    CHECK(!findOpt(map, 4).has_value());
    CHECK_THROWS_AS(static_cast<void>(findExc(map, 4, "Some error msg")),
        std::runtime_error);
};

TEST_SUITE_END();

}

#endif
