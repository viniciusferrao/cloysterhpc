#ifndef CLOYSTERHPC_SCRIPTBUILDER_H_
#define CLOYSTERHPC_SCRIPTBUILDER_H_

#include <vector>
#include <filesystem>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/utils/formatters.h>
#include <cloysterhpc/services/files.h>

namespace cloyster::services {

class ScriptBuilder final
{
    std::vector<std::string> m_commands;
    cloyster::models::OS m_os;
public:
    explicit ScriptBuilder(const cloyster::models::OS& osinfo);

    /**
     * @brief Add a command verbatin to the script, can be used to add comments too
     */
    template<typename... Args>
    constexpr ScriptBuilder& addCommand(fmt::format_string<Args...> fmt, Args&&... args)
    {
        m_commands.emplace_back(fmt::format(fmt, std::forward<decltype(args)>(args)...));
        return *this;
    };

    /**
     * @brief Add a line to a file it does not exists.
     */
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

    /**
     * @brief Create a file from a template (a fmt::format_string) if it has changed. The
     * md5sum of the content is computed and the file is only recreated if the md5sum fails
     */
    template<typename... Args>
    constexpr ScriptBuilder& addFileTemplate(const auto&path, fmt::format_string<Args...> fmt, Args&&... args)
    {
        const std::string content = fmt::format(fmt, std::forward<decltype(args)>(args)...);
        const auto md5sum = cloyster::services::files::md5sum(content);
        addCommand("# Create {} file if checksum fails", path);
        addCommand(R"(echo "{} {}" | md5sum -c --quiet - || cat <<EOF > {}
{}
EOF)",
                   md5sum, path, path, content);

        return *this;
    };

    /**
     * @brief Add a line break
     */
    ScriptBuilder& addNewLine();
    ScriptBuilder& enableService(const std::string_view service);
    ScriptBuilder& disableService(const std::string_view service);
    ScriptBuilder& startService(const std::string_view service);
    ScriptBuilder& stopService(const std::string_view service);
    /**
     * @brief Insert a command to install the package, the command
     * may change depending on m_os
     */
    ScriptBuilder& addPackage(const std::string_view pkg);
    ScriptBuilder& removePackage(const std::string_view pkg);
    ScriptBuilder& removeLineWithKeyFromFile(const std::filesystem::path& path, const std::string& key);
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] const std::vector<std::string>& commands() const;
};

};

#endif
