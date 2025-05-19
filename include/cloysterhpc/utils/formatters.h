
#ifndef CLOYSTERHPC_FORMATTERS_H_
#define CLOYSTERHPC_FORMATTERS_H_


#include <fmt/format.h>
#include <filesystem>

#include <cloysterhpc/models/os.h>

// Custom formatters for 3rd party types
template <>
struct fmt::formatter<std::filesystem::path> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const std::filesystem::path& path, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};

// Custom formatters for our types
template <>
struct fmt::formatter<cloyster::models::OS> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const cloyster::models::OS& osinfo, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "OS(distro={}, kernel={})",
            osinfo.getDistroString(), osinfo.getKernel());
    }
};

#endif


