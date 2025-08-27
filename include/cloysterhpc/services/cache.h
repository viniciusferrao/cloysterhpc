#ifndef CLOYSTERHPC_SERVICES_CACHE_H_
#define CLOYSTERHPC_SERVICES_CACHE_H_

#include <filesystem>
#include <functional>
#include <fmt/core.h>

#include <cloysterhpc/functions.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

namespace cloyster::services::cache::fs {

TEST_SUITE_BEGIN("cloyster::services::cache::fs");

/**
 * @brief Runs the cost intensive function `function` and cache the results into
 * `path`, subsequent calls return the cached content.
 */
std::string run(std::string_view name, const std::filesystem::path& path, std::invocable auto&& function, auto&&... argument)
{
    if (std::filesystem::exists(path)) {
        LOG_INFO("Returning cached result for function from {}", name, path.string());
        return files::read(path);
    }

    LOG_INFO("Running cost intensive function {}, this may take a while ...", name);
    const std::string contents = 
        std::invoke(
            std::forward<decltype(function)>(function),
            std::forward<decltype(argument)>(argument)...);
    LOG_INFO("... finished running {}, caching the results", name);
    LOG_INFO("{}", contents);
    files::write(path, contents);
    return contents;
}

TEST_CASE("run") {
    // Test setup
    const std::filesystem::path testPath = "test/output/cache/cached.txt";
    files::remove(testPath.string());

    // Run the first time, this should create the file
    auto function = []() { return "foo"; };
    const std::string result = run("cacheTest", testPath, function);
    CHECK(result == "foo");
    // Hack the cache to ensure the second call 
    CHECK(functions::exists(testPath));
    files::write(testPath, "bar");
    const std::string result2 = run("cacheTest", testPath, function);
    CHECK(result2 == "bar");
    files::remove(testPath.string());
}

/**
 * @brief Return the cached value or takes the checksum of path, cache and return it
 *   Saves the cache state at [path].checksum
 */
std::string checksum(std::string_view name, const std::filesystem::path& path);

TEST_CASE("checksum") {
    namespace fs = std::filesystem;
    const auto path = fs::path("test/output/cache/");
    const auto testFile = path / "textfile.txt";
    const auto chksFile = fs::path(fmt::format("{}.checksum", testFile.string()));

    files::remove(chksFile);
    CHECK(!files::exists(chksFile));
    files::write(testFile, "foo");
    const auto expectedChecksum = files::checksum(testFile);
    const auto cachedChecksum1 = checksum("test-checksum", testFile);
    CHECK(expectedChecksum == cachedChecksum1);
    CHECK(files::exists(chksFile));
    const auto cachedChecksum2 = files::read(chksFile);
    CHECK(expectedChecksum == cachedChecksum2);
    files::write(chksFile, "this_is_a_checksum");
    const auto cachedChecksum3 = checksum("test-checksum", testFile);
    CHECK(cachedChecksum3 == "this_is_a_checksum");
};

TEST_SUITE_END();

}
#endif
