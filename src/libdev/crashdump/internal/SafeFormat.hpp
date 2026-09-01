#pragma once

#include <cstddef>

namespace CrashDump
{

// Formatting that neither allocates nor consults locale state, so that a report
// can be written from a signal handler, or from a process whose heap is already
// the reason it is dying.
//
// Each function writes into the caller's buffer and returns the number of
// characters written, truncating rather than overflowing. None of them writes a
// terminating null.

std::size_t formatSigned(char* buffer, std::size_t size, long long value);
std::size_t formatUnsigned(char* buffer, std::size_t size, unsigned long long value);

// Lower-case, no "0x" prefix, left-padded with zeroes up to minDigits.
std::size_t formatHex(char* buffer, std::size_t size, unsigned long long value, std::size_t minDigits);

std::size_t formatText(char* buffer, std::size_t size, const char* text);

std::size_t textLength(const char* text);

} // namespace CrashDump
