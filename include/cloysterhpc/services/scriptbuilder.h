#ifndef CLOYSTERHPC_SCRIPTBUILDER_H_
#define CLOYSTERHPC_SCRIPTBUILDER_H_

#include <filesystem>
#include <set>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>

#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/utils/formatters.h>

namespace cloyster::services {

class ScriptBuilder final {
    std::vector<std::string> m_commands;
    cloyster::models::OS m_os;

public:
    explicit ScriptBuilder(const cloyster::models::OS& osinfo);

    /**
     * @brief Add a command verbatin to the script, can be used to add comments
     * too
     */
    template <typename... Args>
    constexpr ScriptBuilder& addCommand(
        fmt::format_string<Args...> fmt, Args&&... args)
    {
        m_commands.emplace_back(
            fmt::format(fmt, std::forward<decltype(args)>(args)...));
        return *this;
    };

    /**
     * @brief Add a line to a file it does not exists.
     */
    template <typename... Args>
    constexpr ScriptBuilder& addLineToFile(const auto& path,
        const std::string_view key, fmt::format_string<Args...> fmt,
        Args&&... args)
    {
        auto line = fmt::format(fmt, std::forward<decltype(args)>(args)...);
        return addCommand("# Adding line to file")
            .addCommand("grep -q \"{key}\" \"{path}\" || \\\n  echo \"{line}\" "
                        ">> \"{path}\"",
                fmt::arg("key", key), fmt::arg("path", path),
                fmt::arg("line", line));
    };

    /**
     * @brief Create a file from a template (a fmt::format_string) if it has
     * changed. The md5sum of the content is computed and the file is only
     * recreated if the md5sum fails
     */
    template <typename... Args>
    constexpr ScriptBuilder& addFileTemplate(
        const auto& path, fmt::format_string<Args...> fmt, Args&&... args)
    {
        const std::string content
            = fmt::format(fmt, std::forward<decltype(args)>(args)...);
        addCommand("# Create {} file", path);
        addCommand(R"(touch {path}
cat <<EOF > {path}
{content}
EOF
)",
            fmt::arg("path", path), fmt::arg("content", content));

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
    ScriptBuilder& addPackages(const std::set<std::string>& pkgs);
    ScriptBuilder& removePackage(const std::string_view pkg);
    ScriptBuilder& removeLineWithKeyFromFile(
        const std::filesystem::path& path, const std::string& key);
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] const std::vector<std::string>& commands() const;
};

};

#endif
