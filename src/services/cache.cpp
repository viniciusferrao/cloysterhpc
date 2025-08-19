#include <cloysterhpc/services/cache.h>

namespace cloyster::services::cache::fs {

std::string checksum(std::string_view name, const std::filesystem::path& path)
{
    auto wrapper = [&]() -> std::string {
        return files::checksum(path);
    };
    return run(name, path / ".checksum", wrapper);
}

};
