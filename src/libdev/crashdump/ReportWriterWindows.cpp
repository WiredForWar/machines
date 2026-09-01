#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/SafeFormat.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace CrashDump
{

namespace
{

HANDLE asFileHandle(std::intptr_t handle)
{
    return reinterpret_cast<HANDLE>(handle);
}

} // namespace

ReportWriter::ReportWriter(const NativeChar* path)
{
    const HANDLE file
        = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file != INVALID_HANDLE_VALUE)
    {
        handle_ = reinterpret_cast<std::intptr_t>(file);
    }
}

ReportWriter::~ReportWriter()
{
    if (isOpen())
    {
        CloseHandle(asFileHandle(handle_));
    }
}

bool ReportWriter::isOpen() const
{
    return handle_ != -1;
}

void ReportWriter::write(const char* text)
{
    write(text, textLength(text));
}

void ReportWriter::write(const char* data, std::size_t size)
{
    if (! isOpen() || size == 0)
    {
        return;
    }

    DWORD written{};
    WriteFile(asFileHandle(handle_), data, static_cast<DWORD>(size), &written, nullptr);
}

void ReportWriter::writeSigned(long long value)
{
    char buffer[32]{};
    write(buffer, formatSigned(buffer, sizeof(buffer), value));
}

void ReportWriter::writeUnsigned(unsigned long long value)
{
    char buffer[32]{};
    write(buffer, formatUnsigned(buffer, sizeof(buffer), value));
}

void ReportWriter::writeAddress(std::uintptr_t value)
{
    char buffer[32]{ '0', 'x' };
    const std::size_t digits = formatHex(buffer + 2, sizeof(buffer) - 2, value, 0);
    write(buffer, digits + 2);
}

void ReportWriter::newLine()
{
    write("\r\n", 2);
}

void ReportWriter::copyFile(const NativeChar* path)
{
    const HANDLE source
        = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (source == INVALID_HANDLE_VALUE)
    {
        return;
    }

    char buffer[4096]{};
    DWORD read{};

    while (ReadFile(source, buffer, sizeof(buffer), &read, nullptr) && read > 0)
    {
        write(buffer, read);
    }

    CloseHandle(source);
}

void announceReport(const NativeChar* path)
{
    OutputDebugStringW(L"Machines: crash report written to ");
    OutputDebugStringW(path);
    OutputDebugStringW(L"\r\n");
}

} // namespace CrashDump
