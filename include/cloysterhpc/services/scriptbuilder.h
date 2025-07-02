#ifndef CLOYSTERHPC_SCRIPTBUILDER_H_
#define CLOYSTERHPC_SCRIPTBUILDER_H_

#include <vector>
#include <filesystem>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/utils/formatters.h>

namespace cloyster::services {

class ScriptBuilder final
{
    std::vector<std::string> m_commands;
    cloyster::models::OS m_os;
public:
    explicit ScriptBuilder(const cloyster::models::OS& osinfo);

    template<typename... Args>
    constexpr ScriptBuilder& addCommand(fmt::format_string<Args...> fmt, Args&&... args)
    {
        m_commands.emplace_back(fmt::format(fmt, std::forward<decltype(args)>(args)...));
        return *this;
    };

    template <typename... Args>
    constexpr ScriptBuilder& addLineToFile(
        const auto& path,
        const std::string_view key,
        fmt::format_string<Args...> fmt,
        Args&&... args)
    {
        auto line = fmt::format(fmt, std::forward<decltype(args)>(args)...);
        return addCommand("grep -q \"{}\" \"{}\" || \\\n  echo \"{}\" >> \"{}\"", key, path, line, path);
    };

    ScriptBuilder& addNewLine();
    ScriptBuilder& enableService(const std::string_view service);
    ScriptBuilder& disableService(const std::string_view service);
    ScriptBuilder& startService(const std::string_view service);
    ScriptBuilder& stopService(const std::string_view service);
    ScriptBuilder& addPackage(const std::string_view pkg);
    ScriptBuilder& removePackage(const std::string_view pkg);
    ScriptBuilder& removeLineWithKeyFromFile(const std::filesystem::path& path, const std::string& key);
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] const std::vector<std::string>& commands() const;
};

};

#endif
