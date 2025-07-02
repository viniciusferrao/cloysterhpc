#ifndef CLOYSTER_TEMPDIR_H
#define CLOYSTER_TEMPDIR_H

#include <filesystem>

/**
 * A class that creates a temporary directory on instantiation, and deletes it
 * when destroyed
 *
 * More or less inspired by Python's `tempfile.TemporaryDirectory` class
 */
class [[deprecated("Use std::filesystem::temp_directory()")]] TempDir {
private:
    std::filesystem::path m_path;

public:
    [[nodiscard]] const std::filesystem::path& name() const;

    TempDir();
    ~TempDir();

    TempDir(TempDir&) = delete;
    TempDir(TempDir&&) = delete;
};

#endif
