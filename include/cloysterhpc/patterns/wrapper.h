#ifndef CLOYSTERHPC_PATTERNS_WRAPPER_H_
#define CLOYSTERHPC_PATTERNS_WRAPPER_H_

namespace cloyster {


/**
 * @class Wrapper
 * @brief Type-safe wrapper for primitive types, providing strong typedef semantics.
 *
 * Wraps a primitive type `T` with a tag `Tag` to create a distinct type, preventing
 * accidental misuse (e.g., mixing `UserId` and `Email`). Optimized to have no
 * runtime overhead, matching the representation of `T`.
 *
 * @tparam T The primitive type to wrap (e.g., `int`, `std::string`).
 * @tparam Tag A unique type (typically a forward-declared struct) to distinguish wrappers.
 *
 * Example usage:
 * @code
 * struct UserIdTag; // Incomplete type
 * struct EmailTag;  // Incomplete type
 * using UserId = cloyster::Wrapper<int, UserIdTag>;
 * using Email = cloyster::Wrapper<std::string, EmailTag>;
 * 
 * UserId id(42);
 * Email email("alice@example.com");
 * 
 * // Type safety: cannot pass Email to function expecting UserId
 * void processUser(UserId user) { int raw = static_cast<int>(user); }
 * processUser(id);        // OK
 * // processUser(email);  // Error: Email is not UserId
 * @endcode
 */
template <typename T, typename Tag>
struct Wrapper {
    T value;
    explicit constexpr Wrapper(const T& val) : value(val) {}
    explicit constexpr operator T() const { return value; }
};

};

#endif
