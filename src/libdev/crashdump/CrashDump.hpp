#pragma once

#include <chrono>
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

// Watch the calling thread for responsiveness. Once timeout passes with no
// intervening heartbeat(), a report holding that thread's call stack is written
// and the process is left running: coverage of the heartbeat is not complete,
// so a false alarm has to cost a file rather than the player's session. A zero
// timeout does nothing.
void startWatchdog(std::chrono::seconds timeout);

// Tell the watchdog that the watched thread is still making progress. One
// relaxed atomic increment, so it is cheap enough for the main loop and for
// every progress report during a load.
void heartbeat();

// Stop the watcher and wait for it to finish. Called by shutdown() as well, so
// that a caller that only wants the watchdog stopped need not take the handlers
// down with it.
void stopWatchdog();

// Restore the previous handlers and stop the watchdog.
void shutdown();

} // namespace CrashDump
