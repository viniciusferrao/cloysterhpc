
/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/cloyster.h>
#include <cloysterhpc/diskImage.h>
#include <cloysterhpc/functions.h>
#include <cloysterhpc/models/os.h>
#include <cloysterhpc/services/files.h>
#include <cloysterhpc/services/log.h>
#include <unordered_map>

// @FIXME: This file need some work
//
// - The ISO can be probed for more information usign isoinfo command
// - The isKnownImage is initializing data, this is a little weird

const std::filesystem::path& DiskImage::getPath() const { return m_path; }

void DiskImage::setPath(const std::filesystem::path& path)
{
    if (path.extension() != ".iso")
        throw std::runtime_error("Disk Image must have ISO extension");

    // Verify checksum only if the image is known.
    if (isKnownImage(path)) {
#ifdef NDEBUG
        if (!hasVerifiedChecksum(path))
            throw std::runtime_error("Disk Image checksum isn't valid");
#endif
    }

    m_path = path;
}

bool DiskImage::isKnownImage(const std::filesystem::path& path)
{
    for (const auto& image : m_knownImageFilename) {
        if (path.filename().string() == image) {
            LOG_TRACE("Disk image is recognized")

            auto imageView = std::string_view(image);
            if (imageView.starts_with("Rocky")) {
                m_distro = cloyster::models::OS::Distro::Rocky;
            } else if (imageView.starts_with("rhel")) {
                m_distro = cloyster::models::OS::Distro::RHEL;
            } else if (imageView.starts_with("OracleLinux")) {
                m_distro = cloyster::models::OS::Distro::OL;
            } else if (imageView.starts_with("AlmaLinux")) {
                m_distro = cloyster::models::OS::Distro::AlmaLinux;
            } else {
                throw std::logic_error(fmt::format(
                    "Can't determine the distro for the image {}", image));
            }

            return true;
        }
    }

    LOG_TRACE("Disk image is unknown. Maybe you're using a custom image or "
              "changed the default name?");
    return false;
}

cloyster::models::OS::Distro DiskImage::getDistro() const
{
    LOG_ASSERT(m_distro.has_value(), "Trying to getDistro() uninitialized");
    return m_distro.value();
}

// BUG: Consider removing/reimplement this method
bool DiskImage::hasVerifiedChecksum(const std::filesystem::path& path)
{
    if (cloyster::dryRun) {
        LOG_INFO("Dry Run: Would verify disk image checksum.")
        return true;
    }

    LOG_INFO("Verifying disk image checksum... This may take a while, use "
             "`--skip disk-checksum` to skip")
    if (cloyster::shouldSkip("disk-checksum")) {
        LOG_WARN(
            "Skiping disk the image checksum because `--skip disk-checksum`");
        return true;
    }

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

    auto checksum = cloyster::services::files::checksum(path);
    LOG_INFO("SHA256 checksum of file {} is: {}", path.string(), checksum);

    if (checksum == hash_map.find(path.filename().string())->second) {
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
    /*
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
    */
}
