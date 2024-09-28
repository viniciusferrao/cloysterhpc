#include <array>
#include <cloysterhpc/tempdir.h>
#include <cstdint>
#include <fmt/format.h>
#include <random>

static char value_to_char(std::uint64_t value)
{
    if (value < 10) {
        return char(int('0') + value);
    }

    if (value < 36) {
        return char(int('a') + (value - 10));
    }

    return 'A';
}

#define NOSONAR(code) code

#pragma warning disable S2245
static std::filesystem::path create_temporary_filename()
{
    static std::random_device dev;
    std::mt19937 rng(dev()); // NOSONAR
    NOSONAR(std::uniform_int_distribution<std::mt19937::result_type>)
    charbyte(0, 36);

    std::array<char, 8> values = { value_to_char(charbyte(rng)),
        value_to_char(charbyte(rng)), value_to_char(charbyte(rng)),
        value_to_char(charbyte(rng)), value_to_char(charbyte(rng)),
        value_to_char(charbyte(rng)), value_to_char(charbyte(rng)), '\0' };

    std::string basename = fmt::format("temp{}", values.data());
    return std::filesystem::path { "/tmp" } / basename; // NOSONAR
}
#pragma warning restore S2245

TempDir::TempDir()
{
    auto path = create_temporary_filename();
    std::filesystem::create_directory(path);
    m_path = path;
}

TempDir::~TempDir() { std::filesystem::remove_all(m_path); }

const std::filesystem::path& TempDir::name() const { return m_path; }
