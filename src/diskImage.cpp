/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cloysterhpc/os.h"
#include "cloysterhpc/services/log.h"
#include <cloysterhpc/diskImage.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

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
    for (const auto& image : m_knownImageFilename)
        if (path.filename().string() == image) {
            LOG_TRACE("Disk image is recognized");
            return true;
        }

    LOG_TRACE("Disk image is unknown. Maybe you're using a custom image or "
              "changed the default name?");
    return false;
}

bool DiskImage::hasVerifiedChecksum(const std::filesystem::path& path)
{
    if (!isKnownImage(path)) {
        LOG_TRACE("Disk image is unknown. Can't verify checksum");
        return false;
    }

    LOG_TRACE("Verifying disk image checksum... This may take a while");

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
    };

    CryptoPP::SHA256 hash;
    std::string isoHash = hash_map.find(path.filename().string())->second;
    std::string output;
    auto sink = std::make_unique<CryptoPP::StringSink>(output);
    auto encoder = std::make_unique<CryptoPP::HexEncoder>(sink.get());
    auto filter = std::make_unique<CryptoPP::HashFilter>(hash, encoder.get());

    CryptoPP::FileSource(path.string().c_str(), true, filter.get(), true);
    transform(output.begin(), output.end(), output.begin(), ::tolower);

    sink.release();
    encoder.release();
    filter.release();

    if (output == isoHash) {
        LOG_TRACE("Checksum - The disk image is valid");
        return true;
    }

    LOG_TRACE("Checksum - The disk image is invalid. Maybe you're using a "
              "custom image?");
    return false;
}
