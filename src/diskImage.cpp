
/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/diskImage.h>
#include <cloysterhpc/os.h>
#include <cloysterhpc/services/log.h>
#include <cstddef>
#include <fstream>
#include <glibmm/checksum.h>
#include <ios>
#include <istream>
#include <unordered_map>
#include <vector>

const std::filesystem::path& DiskImage::getPath() const { return m_path; }

void DiskImage::setPath(const std::filesystem::path& path)
{
    if (path.extension() != ".iso")
        throw std::runtime_error("Disk Image must have ISO extension");

    // Verify checksum only if the image is known.
    if (isKnownImage(path)) {
        if (!hasVerifiedChecksum(path))
            throw std::runtime_error("Disk Image checksum isn't valid");
    }

    m_path = path;
}

bool DiskImage::isKnownImage(const std::filesystem::path& path)
{
    for (const auto& image : m_knownImageFilename) {
        if (path.filename().string() == image) {
            LOG_TRACE("Disk image is recognized")
            return true;
        }
    }

    LOG_TRACE("Disk image is unknown. Maybe you're using a custom image or "
              "changed the default name?");
    return false;
}

// BUG: Consider removing/reimplement this method
bool DiskImage::hasVerifiedChecksum(const std::filesystem::path& path)
{
    if (!isKnownImage(path)) {
        LOG_TRACE("Disk image is unknown. Can't verify checksum")
        return false;
    }

    LOG_TRACE("Verifying disk image checksum... This may take a while")

    // BUG: This should no be hardcoded here. An ancillary file should be used
    std::unordered_map<std::string, std::string> hash_map = {
        { "rhel-8.8-x86_64-dvd.iso",
            "517abcc67ee3b7212f57e180f5d30be3e8269e7a99e127a3399b7935c7e00a0"
            "9" },
        { "OracleLinux-R8-U8-x86_64-dvd.iso",
            "cae39116245ff7c3c86d5305d9c11430ce5c4e512987563435ac59c37a082d7"
            "e" },
        { "Rocky-8.8-x86_64-dvd1.iso",
            "7b8bdfe189cf24ae5c2d6a88f7a0b5f3012d23f9332c47943d538b4bc03a370"
            "4" },
        { "AlmaLinux-8.8-x86_64-dvd.iso",
            "635b30b967b509a32a1a3d81401db9861922acb396d065922b39405a43a04a3"
            "1" },
        { "Rocky-9.5-x86_64-dvd.iso",
            "ba60c3653640b5747610ddfb4d09520529bef2d1d83c1feb86b0c84dff31e04"
            "e" }
    };

    Glib::Checksum checksum(Glib::Checksum::Type::SHA256);

    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::filesystem::filesystem_error(
            "Failed to open file", path, std::error_code());
    }

    // Read the file in chunks of 16834 bytes
    constexpr std::size_t chunk_size = 16384;
    std::vector<std::byte> buffer(chunk_size);

    while (file.read(reinterpret_cast<std::istream::char_type*>(buffer.data()),
        static_cast<std::streamsize>(buffer.size()))) {
        std::streamsize bytesRead = file.gcount();

        checksum.update(
            reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead);
    }

    // Handle any leftover bytes after the while loop ends
    std::streamsize bytesRead = file.gcount();
    if (bytesRead > 0) {
        checksum.update(
            reinterpret_cast<const unsigned char*>(buffer.data()), bytesRead);
    }

    LOG_INFO(fmt::format("SHA256 checksum of file {} is: {}", path.string(),
        checksum.get_string()));

    if (checksum.get_string()
        == hash_map.find(path.filename().string())->second) {
        LOG_TRACE("Checksum - The disk image is valid")
        return true;
    }

    LOG_TRACE("Checksum - The disk image is invalid. Maybe you're using a "
              "custom image?");
    return false;
}

#ifdef BUILD_TESTING
#include <doctest/doctest.h>
#else
#define DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>
#endif

TEST_SUITE("Disk image test suite")
{
    DiskImage diskImage;
    const auto path = std::filesystem::current_path() / "/sample/checksum.iso";

    TEST_CASE("Verify if is unknown image")
    {
        REQUIRE_FALSE(diskImage.isKnownImage(path));
    }

    TEST_CASE("Verify invalid checksum")
    {
        REQUIRE_FALSE(diskImage.hasVerifiedChecksum(path));
    }
}
