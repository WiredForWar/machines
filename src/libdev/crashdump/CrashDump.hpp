#pragma once

#include <filesystem>
#include <string_view>

namespace CrashDump
{

// Install the crash handlers and direct their output at dumpDirectory, creating
// it if it does not exist. Call this as early in main() as it will go: a crash
// before the call is reported by the operating system and by nothing else.
void initialize(const std::filesystem::path& dumpDirectory);

// Record which application a report came from and which build of it. Both
// executables write their reports into the same directory, so the name is what
// says which of them produced one. Any field left empty is left out of the
// report, and a report written before this is called omits all three.
void setApplicationInfo(std::string_view name, std::string_view version, std::string_view commitHash);

// Restore the previous handlers.
void shutdown();

} // namespace CrashDump
