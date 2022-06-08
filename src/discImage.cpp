/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "discImage.h"

const std::filesystem::path& DiscImage::getPath() const {
    return m_path;
}

void DiscImage::setPath(const std::filesystem::path& path) {
    if (path.extension() != ".iso")
        throw std::runtime_error("Disc Image must have ISO extension");

    m_path = path;
}

bool DiscImage::isKnownImage() {
    for (const auto& image : m_knownImageFilename)
        if (m_path.filename() == image)
            return true;

    return false;
}
