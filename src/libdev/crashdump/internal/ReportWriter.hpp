#pragma once

#include <cstddef>
#include <cstdint>

namespace CrashDump
{

// The character a path is spelled in by the platform's own file API, so that a
// dump can be written into a directory whose name is not representable in the
// local code page.
#ifdef _WIN32
using NativeChar = wchar_t;
#else
using NativeChar = char;
#endif

// An append-only text sink that allocates nothing, takes no lock and uses only
// the raw operating system calls that stay safe once a process is dying or has
// been stopped mid-frame.
//
// A writer that could not open its file is inactive rather than invalid: every
// call on it succeeds and does nothing, so a handler needs no error path.
class ReportWriter
{
public:
    // Creates the file, truncating an existing one.
    explicit ReportWriter(const NativeChar* path);
    ~ReportWriter();

    ReportWriter(const ReportWriter&) = delete;
    ReportWriter& operator=(const ReportWriter&) = delete;

    bool isOpen() const;

    void write(const char* text);
    void write(const char* data, std::size_t size);
    void writeSigned(long long value);
    void writeUnsigned(unsigned long long value);

    // Written with a "0x" prefix.
    void writeAddress(std::uintptr_t value);

    // The platform's own line ending, so that the file opens cleanly in the
    // editor the person reading it is likely to have.
    void newLine();

    // Copies an existing file's contents in, for the memory map a report needs
    // in order to be rebased against a position-independent binary. Does
    // nothing where there is no such file.
    void copyFile(const NativeChar* path);

private:
    std::intptr_t handle_{ -1 };
};

// Report the path of a written dump to whatever is listening: a debugger on
// Windows, standard error elsewhere. The game is a windowed-subsystem binary on
// Windows and has no console to print to.
void announceReport(const NativeChar* path);

} // namespace CrashDump
