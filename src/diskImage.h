/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_DISKIMAGE_H_
#define CLOYSTERHPC_DISKIMAGE_H_

#include <array>
#include <filesystem>

class DiskImage {
private:
    std::filesystem::path m_path;
    static constexpr auto m_knownImageFilename { std::to_array<const char*>(
        { "rhel-8.5-x86_64-dvd.iso", "OracleLinux-R8-U5-x86_64-dvd.iso",
            "Rocky-8.5-x86_64-dvd1.iso" }) };
    bool isKnownImage();

public:
    const std::filesystem::path& getPath() const;
    void setPath(const std::filesystem::path& path);
};

#endif // CLOYSTERHPC_DISKIMAGE_H_
