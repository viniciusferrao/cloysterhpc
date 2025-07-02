#ifndef CLOYSTER_UTILS_STRING_H
#define CLOYSTER_UTILS_STRING_H

#include <algorithm>
#include <string>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> // For boost::is_any_of
#include <magic_enum/magic_enum.hpp>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::utils::string {

std::string upper(auto str)
{
    std::ranges::transform(str, str.begin(),
                           [](unsigned char chr){ return std::toupper(chr); } // correct
                  );
    return str;
}

std::string lower(auto str)
{
    std::ranges::transform(str, str.begin(),
                           [](unsigned char chr){ return std::tolower(chr); } // correct
                  );
    return str;
}

std::string rstrip(auto str, const auto& trailing = " ") {
    auto pos = str.find_last_not_of(trailing);
    if (pos == std::string_view::npos) {
        return ""; // String is all trailing characters
    }
    return std::string(str.substr(0, pos + 1));
}

std::vector<std::string> split(const auto& content, std::string_view separator) {
    std::vector<std::string> result;

    if (separator.empty()) {
        result.emplace_back(content);
        return result;
    }
    boost::algorithm::split(result, content, boost::is_any_of(separator));

    return result;
}

TEST_SUITE_BEGIN("cloyster::utils::string::split");

TEST_CASE("split: Basic splitting with comma") {
    std::string content = "apple,banana,orange";
    std::string separator = ",";
    std::vector<std::string> expected = {"apple", "banana", "orange"};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Splitting with multi-character separator") {
    std::string content = "one--two---three";
    std::string separator = "--";
    // Note: boost::is_any_of treats each char in separator as a delimiter.
    // So "one--two" split by "--" will result in "one", "", "two".
    std::vector<std::string> expected = {"one", "", "two", "", "", "three"};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Leading and trailing separators") {
    std::string content = ",start,middle,end,";
    std::string separator = ",";
    std::vector<std::string> expected = {"", "start", "middle", "end", ""};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Consecutive separators") {
    std::string content = "part1,,part2";
    std::string separator = ",";
    std::vector<std::string> expected = {"part1", "", "part2"};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: No separator found") {
    std::string content = "singleword";
    std::string separator = ",";
    std::vector<std::string> expected = {"singleword"};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Empty content string") {
    std::string content = "";
    std::string separator = ",";
    std::vector<std::string> expected = {""};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Empty separator string") {
    std::string content = "test";
    std::string separator = "";
    std::vector<std::string> expected = {"test"}; // As per the custom handling in the function
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Content with only separators") {
    std::string content = ",,,";
    std::string separator = ",";
    std::vector<std::string> expected = {"", "", "", ""};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Content with only one character and that is the separator") {
    std::string content = ",";
    std::string separator = ",";
    std::vector<std::string> expected = {"", ""};
    CHECK(split(content, separator) == expected);
}

TEST_CASE("split: Content with a single character, not a separator") {
    std::string content = "a";
    std::string separator = ",";
    std::vector<std::string> expected = {"a"};
    CHECK(split(content, separator) == expected);
}

TEST_SUITE_END();

}// namespace string

#endif
