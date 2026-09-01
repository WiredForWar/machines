#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace CrashDump
{

// A captured call stack. The frames are raw instruction addresses: a release
// binary is stripped, so resolving them to names happens offline against the
// debug information split out at build time.
struct StackTrace
{
    static constexpr std::size_t maxFrames{ 128 };

    std::array<std::uintptr_t, maxFrames> frames{};
    std::size_t frameCount{};
};

// Capture the calling thread's stack, discarding the innermost skipFrames so
// that the capture machinery itself does not appear in the report.
void captureStackTrace(StackTrace& trace, std::size_t skipFrames);

// Capture the stack a platform context record describes, leaving the context
// unmodified. Both arguments are opaque and platform-defined:
//
//   POSIX    context is the ucontext_t handed to an SA_SIGINFO handler, and
//            thread is ignored.
//   Windows  context is a CONTEXT, and thread is the HANDLE of the thread it
//            belongs to, which must be the current thread or a suspended one.
void captureStackTraceFromContext(StackTrace& trace, void* context, void* thread);

// Do whatever the unwinder would otherwise do lazily on its first use, at a
// point where allocating and taking a loader lock are still safe. Call once
// from ordinary code before any handler can run.
void warmUpStackWalk();

// Remember the calling thread, so that another thread can capture its stack
// later. Call on the thread that is to be watched.
void rememberWatchedThread();

// Capture the stack of the thread rememberWatchedThread() was called on, from
// a different thread. The watched thread is held still for as long as it takes
// and left running afterwards. Answers false if there is no watched thread, or
// if it could not be held still, or if it did not respond in time.
bool captureWatchedThreadStackTrace(StackTrace& trace);

} // namespace CrashDump
