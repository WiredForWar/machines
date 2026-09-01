#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/SafeFormat.hpp"

#include <fcntl.h>
#include <unistd.h>

namespace CrashDump
{

ReportWriter::ReportWriter(const NativeChar* path)
{
    const int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (file >= 0)
    {
        handle_ = file;
    }
}

ReportWriter::~ReportWriter()
{
    if (isOpen())
    {
        close(static_cast<int>(handle_));
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

    std::size_t offset{};

    // A short write is normal on a signal-interrupted descriptor, so the loop
    // is what makes the report complete rather than merely likely.
    while (offset < size)
    {
        const ssize_t written = ::write(static_cast<int>(handle_), data + offset, size - offset);

        if (written <= 0)
        {
            return;
        }

        offset += static_cast<std::size_t>(written);
    }
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
    write("\n", 1);
}

void ReportWriter::copyFile(const NativeChar* path)
{
    const int source = open(path, O_RDONLY);

    if (source < 0)
    {
        return;
    }

    char buffer[4096]{};
    ssize_t read{};

    while ((read = ::read(source, buffer, sizeof(buffer))) > 0)
    {
        write(buffer, static_cast<std::size_t>(read));
    }

    close(source);
}

void announceReport(const NativeChar* path)
{
    const char prefix[]{ "Machines: crash report written to " };

    ::write(STDERR_FILENO, prefix, sizeof(prefix) - 1);
    ::write(STDERR_FILENO, path, textLength(path));
    ::write(STDERR_FILENO, "\n", 1);
}

} // namespace CrashDump
