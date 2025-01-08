/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_DISKIMAGE_H_
#define CLOYSTERHPC_DISKIMAGE_H_

#include <array>
#include <filesystem>

/**
 * @class DiskImage
 * @brief Manages disk image paths and validation for known images.
 *
 * The DiskImage class provides functionality to handle disk image paths,
 * verify if a disk image is known, and check if it has a verified checksum.
 */
class DiskImage {
private:
    std::filesystem::path m_path;

    /**
     * @brief List of known disk image filenames.
     */
    static constexpr auto m_knownImageFilename { std::to_array<const char*>(
        { "rhel-8.8-x86_64-dvd.iso", "OracleLinux-R8-U8-x86_64-dvd.iso",
            "Rocky-8.8-x86_64-dvd1.iso", "AlmaLinux-8.8-x86_64-dvd.iso" }) };

public:
    const std::filesystem::path& getPath() const;
    void setPath(const std::filesystem::path& path);

    /**
     * @brief Checks if the given disk image is known.
     *
     * @param path Filesystem path to the disk image to check.
     * @return True if the disk image is known, false otherwise.
     */
    static bool isKnownImage(const std::filesystem::path& path);
    
    /**
     * @brief Checks if the given disk image has a verified checksum.
     *
     * @param path Filesystem path to the disk image to check.
     * @return True if the disk image has a verified checksum, false otherwise.
     */
    bool hasVerifiedChecksum(const std::filesystem::path& path);
};

#endif // CLOYSTERHPC_DISKIMAGE_H_
