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
 * struct UserTag; // Incomplete type
 * struct EmailTag;  // Incomplete type
 * using User = Wrapper<std::string, UserIdTag>;
 * using Email = Wrapper<std::string, EmailTag>;
 * 
 * User user("alice");
 * Email email("alice@example.com");
 * 
 * // Type safety: cannot pass Email to function expecting UserId
 * void processUser(User user) {}
 * processUser(user);  // OK
 * processUser(email); // Error: Email is not User
 * @endcode
 */
template <typename T, typename Tag>
class Wrapper final {
    T value;
public:
    constexpr explicit Wrapper(T val) noexcept : value(std::move(val)) {}
    Wrapper() = delete;
    Wrapper(const Wrapper&) = delete;
    Wrapper(const Wrapper&&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;
    Wrapper& operator=(Wrapper&&) = delete;
    ~Wrapper() = default;

    // Implicit conversion to T is desired in this case, so I disabled the linter
    // in the next line
    constexpr operator T() const { return value; } // NOLINT
    const T& get() const { return value; }
};

/** @namespace wrappers
 * @brief Namespace for wrapper types.
 * @details This namespace contains wrapper types for common types, such as `std::string`,
 *   `std::filesystem::path`, and `int`. These wrappers provide strong typedef semantics
 *   to avoid swapping parameters of the same type in functions calls, (among other problems)
 *   making the code more readable and maintainable.
 */
namespace wrappers {
    namespace fs = std::filesystem;

    struct DestinationPathTag;
    using DestinationPath = Wrapper<fs::path, DestinationPathTag>;

    struct SourcePathTag;
    using SourcePath = Wrapper<fs::path, SourcePathTag>;

    struct ExtensionTag;
    using Extension = Wrapper<std::string, ExtensionTag>;
}

} // namespace cloyster

#endif
