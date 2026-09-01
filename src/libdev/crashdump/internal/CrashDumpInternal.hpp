#pragma once

#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/StackWalk.hpp"

#include <cstddef>
#include <string>

namespace CrashDump
{

// Long enough for any dump directory a launcher is going to choose, and short
// enough to sit on the stack of a handler that may be running on the small
// alternate stack a stack overflow leaves it.
constexpr std::size_t maxReportPath{ 1024 };

// Captured before any handler can run, so that reading it from one needs
// neither allocation nor a lock.
const std::string& appName();
const std::string& appVersion();
const std::string& appCommitHash();

// Compose the path of a report file, as
// <dump directory>/<kind>_<process id>_<seconds since epoch>.<extension>.
// Returns false, leaving path unusable, if the result would not fit.
bool buildReportPath(NativeChar* path, std::size_t size, const char* kind, const char* extension);

// The build a report came from. Omitted entirely when it was never set.
void writeBuildInfo(ReportWriter& writer);

// One raw frame address per line, innermost first.
void writeStackTrace(ReportWriter& writer, const StackTrace& trace);

// Where the image was loaded, for a report whose addresses have to be rebased
// against it. Writes nothing where the platform records that elsewhere.
void writeModuleInfo(ReportWriter& writer);

// Implemented once per platform.
unsigned long long currentProcessId();
void installHandlers();
void uninstallHandlers();

} // namespace CrashDump
