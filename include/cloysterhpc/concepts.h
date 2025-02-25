#ifndef CLOYSTER_CONCEPTS_H
#define CLOYSTER_CONCEPTS_H

#include <concepts>
#include <expected>
#include <iostream>

namespace cloyster::concepts {
// @TODO: Move this to its own file
template <typename T>
concept IsCopyable
    = std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

template <typename T>
concept IsMoveable
    = std::is_move_constructible_v<T> && std::is_move_assignable_v<T>;

template <typename T>
concept NotCopiableNotMoveable = !IsMoveable<T> && !IsCopyable<T>;
/**
 * @brief IsParser<P, T> means: P can parse and unparse Ts from streams. The
 * parsers are allowed to throw exceptions
 */
template <typename Parser_, typename I, typename O>
concept IsParser = requires(Parser_ parser, const I& parseInput, O& parseOutput,
    const O& unparseInput, I& unparseOutput) {
    parser.parse(parseInput, parseOutput);
    parser.unparse(unparseInput, unparseOutput);
};

// @TODO: Move this to its own file
/**
 * @brief ParserNoExc<P, T, E> means: P can parse and unparse Ts from streams.
 * The parsers are not allowed to throw exceptions, it must return errors of
 * type E
 */
template <typename Parser_, typename T, typename E>
concept ParserNoExc = requires(Parser_ parser, std::istream& input,
    std::ostream& output, const T& parsed) {
    { parser.parse(input) } noexcept -> std::same_as<std::expected<T, E>>;
    {
        parser.unparse(parsed, output)
    } noexcept -> std::same_as<std::expected<void, E>>;
};

// @TODO: Move this to its own file
/**
 * @brief Stored<T> means that T represends data in a disk that need to be
 * saved and restored after change.
 */
template <typename File_>
concept IsSaveable = requires(File_ file) {
    { file.load() } -> std::same_as<void>;
    { file.save() } -> std::same_as<void>;
};

}

#endif // CLOYSTER_CONCEPTS_H
