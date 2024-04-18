/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_DISKIMAGE_H_
#define CLOYSTERHPC_DISKIMAGE_H_

#include "functions.h"
#include <array>
#include <filesystem>

class DiskImage {
private:
    std::filesystem::path m_path;
    static constexpr auto m_knownImageFilename { std::to_array<const char*>(
        { "rhel-8.8-x86_64-dvd.iso", "OracleLinux-R8-U8-x86_64-dvd.iso",
            "Rocky-8.8-x86_64-dvd1.iso", "AlmaLinux-8.8-x86_64-dvd.iso" }) };

public:
    bool isKnownImage();
    bool hasVerifiedChecksum();
    [[nodiscard]] const std::filesystem::path& getPath() const;

    template <typename FilePath> void setPath(FilePath&& path)
    {
        std::filesystem::path formattedPath { cloyster::handlePath(
            std::forward<FilePath>(path)) };

        if (formattedPath.extension() != ".iso")
            throw std::runtime_error("Disk Image must have ISO extension");

        m_path = formattedPath;

        // Verify checksum only if the image is known.
        if (isKnownImage()) {
            if (!hasVerifiedChecksum())
                throw std::runtime_error("Disk Image checksum isn't valid");
        }
    }

    DiskImage() = default;
    template <typename FilePath>
        requires(!std::is_same_v<std::decay_t<FilePath>, DiskImage>)
    explicit DiskImage(FilePath&& path)
    {
        setPath(cloyster::handlePath(std::forward<FilePath>(path)));
    }
};

#endif // CLOYSTERHPC_DISKIMAGE_H_
