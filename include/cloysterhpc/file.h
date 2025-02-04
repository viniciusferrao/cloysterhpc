/*
 * Copyright 2025 Arthur Mendes <arthurmco@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_FILE_H_
#define CLOYSTERHPC_FILE_H_

#include <filesystem>

/**
 * Generic file class
 *
 * This class should only read and write,
 */
class GenericFile {
protected:
    std::filesystem::path m_path;

public:
    explicit GenericFile(const std::filesystem::path& path)
        : m_path(path)
    {
    }

    virtual void read() { }
    virtual void write() { }

    virtual ~GenericFile() = default;
};

#endif
