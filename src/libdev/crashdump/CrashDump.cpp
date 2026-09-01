#include "crashdump/CrashDump.hpp"

#include "crashdump/internal/CrashDumpInternal.hpp"
#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/SafeFormat.hpp"
#include "crashdump/internal/StackWalk.hpp"

#include <array>
#include <string>

#include <ctime>

namespace CrashDump
{

namespace
{

// The directory, already terminated by a separator and already in the character
// type the platform's file API takes, so that composing a report path inside a
// handler is a copy and some digits rather than a conversion.
std::array<NativeChar, maxReportPath> dumpPathPrefix_{};
std::size_t dumpPathPrefixLength_{};

std::string appName_;
std::string appVersion_;
std::string appCommitHash_;

bool initialized_{};

std::size_t appendNative(NativeChar* path, std::size_t size, std::size_t offset, const char* text)
{
    while (*text != '\0' && offset < size)
    {
        path[offset] = static_cast<NativeChar>(*text);
        ++offset;
        ++text;
    }

    return offset;
}

std::size_t appendNumber(NativeChar* path, std::size_t size, std::size_t offset, unsigned long long value)
{
    char digits[32]{};
    const std::size_t digitCount = formatUnsigned(digits, sizeof(digits), value);

    for (std::size_t i = 0; i < digitCount && offset < size; ++i)
    {
        path[offset] = static_cast<NativeChar>(digits[i]);
        ++offset;
    }

    return offset;
}

} // namespace

const std::string& appName()
{
    return appName_;
}

const std::string& appVersion()
{
    return appVersion_;
}

const std::string& appCommitHash()
{
    return appCommitHash_;
}

bool buildReportPath(NativeChar* path, std::size_t size, const char* kind, const char* extension)
{
    if (size == 0)
    {
        return false;
    }

    std::size_t offset{};

    for (; offset < dumpPathPrefixLength_ && offset < size; ++offset)
    {
        path[offset] = dumpPathPrefix_[offset];
    }

    offset = appendNative(path, size, offset, kind);
    offset = appendNative(path, size, offset, "_");
    offset = appendNumber(path, size, offset, currentProcessId());
    offset = appendNative(path, size, offset, "_");
    offset = appendNumber(path, size, offset, static_cast<unsigned long long>(std::time(nullptr)));
    offset = appendNative(path, size, offset, ".");
    offset = appendNative(path, size, offset, extension);

    if (offset >= size)
    {
        return false;
    }

    path[offset] = static_cast<NativeChar>('\0');

    return true;
}

void writeBuildInfo(ReportWriter& writer)
{
    if (! appName_.empty())
    {
        writer.write("Application: ");
        writer.write(appName_.data(), appName_.size());
        writer.newLine();
    }

    if (! appVersion_.empty())
    {
        writer.write("Version: ");
        writer.write(appVersion_.data(), appVersion_.size());
        writer.newLine();
    }

    if (! appCommitHash_.empty())
    {
        writer.write("Commit: ");
        writer.write(appCommitHash_.data(), appCommitHash_.size());
        writer.newLine();
    }
}

void writeStackTrace(ReportWriter& writer, const StackTrace& trace)
{
    if (trace.frameCount == 0)
    {
        writer.write("  (no frames recovered)");
        writer.newLine();
        return;
    }

    for (std::size_t i = 0; i < trace.frameCount; ++i)
    {
        writer.write("  ");
        writer.writeAddress(trace.frames[i]);
        writer.newLine();
    }
}

void initialize(const std::filesystem::path& dumpDirectory)
{
    if (initialized_)
    {
        return;
    }

    std::error_code error;
    std::filesystem::create_directories(dumpDirectory, error);

    // path::native() is already spelled in the platform's own character type,
    // which is the whole reason the prefix can be copied rather than converted
    // once a handler is running.
    const std::basic_string<NativeChar>& directory = dumpDirectory.native();

    dumpPathPrefixLength_ = directory.size();

    if (dumpPathPrefixLength_ + 1 >= dumpPathPrefix_.size())
    {
        dumpPathPrefixLength_ = 0;
    }
    else
    {
        for (std::size_t i = 0; i < dumpPathPrefixLength_; ++i)
        {
            dumpPathPrefix_[i] = directory[i];
        }

        dumpPathPrefix_[dumpPathPrefixLength_] = static_cast<NativeChar>(std::filesystem::path::preferred_separator);
        ++dumpPathPrefixLength_;
    }

    warmUpStackWalk();
    installHandlers();

    initialized_ = true;
}

void setApplicationInfo(std::string_view name, std::string_view version, std::string_view commitHash)
{
    appName_ = name;
    appVersion_ = version;
    appCommitHash_ = commitHash;
}

void shutdown()
{
    if (! initialized_)
    {
        return;
    }

    stopWatchdog();
    uninstallHandlers();

    initialized_ = false;
}

} // namespace CrashDump
