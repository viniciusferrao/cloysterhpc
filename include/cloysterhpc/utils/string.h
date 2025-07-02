#ifndef CLOYSTER_UTILS_STRING_H
#define CLOYSTER_UTILS_STRING_H

#include <algorithm>
#include <magic_enum/magic_enum.hpp>
#include <string>

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

} // namespace string

#endif
