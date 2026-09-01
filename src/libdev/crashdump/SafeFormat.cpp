#include "crashdump/internal/SafeFormat.hpp"

namespace CrashDump
{

namespace
{

// The longest decimal representation of a 64-bit value, plus a sign.
constexpr std::size_t maxDigits{ 24 };

std::size_t copyReversed(char* buffer, std::size_t size, const char* digits, std::size_t digitCount)
{
    std::size_t written{};

    for (std::size_t i = digitCount; i > 0 && written < size; --i)
    {
        buffer[written] = digits[i - 1];
        ++written;
    }

    return written;
}

} // namespace

std::size_t formatUnsigned(char* buffer, std::size_t size, unsigned long long value)
{
    char digits[maxDigits]{};
    std::size_t digitCount{};

    if (value == 0)
    {
        digits[digitCount] = '0';
        ++digitCount;
    }

    while (value > 0 && digitCount < maxDigits)
    {
        digits[digitCount] = static_cast<char>('0' + (value % 10));
        ++digitCount;
        value /= 10;
    }

    return copyReversed(buffer, size, digits, digitCount);
}

std::size_t formatSigned(char* buffer, std::size_t size, long long value)
{
    std::size_t written{};

    // Negating the most negative value would overflow, so the magnitude is
    // taken in the unsigned domain where it is always representable.
    unsigned long long magnitude{};

    if (value < 0)
    {
        if (size > 0)
        {
            buffer[0] = '-';
            written = 1;
        }

        magnitude = 0ULL - static_cast<unsigned long long>(value);
    }
    else
    {
        magnitude = static_cast<unsigned long long>(value);
    }

    return written + formatUnsigned(buffer + written, size - written, magnitude);
}

std::size_t formatHex(char* buffer, std::size_t size, unsigned long long value, std::size_t minDigits)
{
    static const char hexDigits[]{ "0123456789abcdef" };

    char digits[maxDigits]{};
    std::size_t digitCount{};

    while (value > 0 && digitCount < maxDigits)
    {
        digits[digitCount] = hexDigits[value & 0xf];
        ++digitCount;
        value >>= 4;
    }

    while (digitCount < minDigits && digitCount < maxDigits)
    {
        digits[digitCount] = '0';
        ++digitCount;
    }

    if (digitCount == 0)
    {
        digits[digitCount] = '0';
        ++digitCount;
    }

    return copyReversed(buffer, size, digits, digitCount);
}

std::size_t formatText(char* buffer, std::size_t size, const char* text)
{
    if (text == nullptr)
    {
        return 0;
    }

    std::size_t written{};

    while (text[written] != '\0' && written < size)
    {
        buffer[written] = text[written];
        ++written;
    }

    return written;
}

std::size_t textLength(const char* text)
{
    if (text == nullptr)
    {
        return 0;
    }

    std::size_t length{};

    while (text[length] != '\0')
    {
        ++length;
    }

    return length;
}

} // namespace CrashDump
