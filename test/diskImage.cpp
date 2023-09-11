/*
 * Created by Lucas Gracioso <contact@lbgracioso.net>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/diskImage.h>
#include <doctest/doctest.h>

TEST_SUITE("Disk image test suite")
{
    DiskImage diskImage;
    const auto path = std::filesystem::current_path() / "sample/checksum.iso";

    TEST_CASE("Verify if is unknown image")
    {
        REQUIRE_FALSE(diskImage.isKnownImage(path));
    }

    TEST_CASE("Verify invalid checksum")
    {
        REQUIRE_FALSE(diskImage.hasVerifiedChecksum(path));
    }
}
