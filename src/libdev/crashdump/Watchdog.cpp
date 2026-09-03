#include "crashdump/CrashDump.hpp"

#include "crashdump/internal/CrashDumpInternal.hpp"
#include "crashdump/internal/ReportWriter.hpp"
#include "crashdump/internal/StackWalk.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace CrashDump
{

namespace
{

// A counter rather than a timestamp, so that the watched thread's side of this
// is a single relaxed store and can sit in the main loop without being thought
// about. Only the watcher cares what time it is.
std::atomic<unsigned long long> beats_{};

// The watcher spends nearly all its life waiting, so a stop has to be able to
// interrupt that wait rather than wait it out.
std::mutex stopMutex_;
std::condition_variable stopRequested_;
bool watching_{}; // guarded by stopMutex_

std::thread watcher_;
std::chrono::seconds timeout_{};

// A genuine deadlock would otherwise write a report every timeout for as long
// as the player leaves the window open. Enough reports to show whether the
// stack is moving, and then quiet.
constexpr int maxReports{ 5 };

void writeHangReport(int reportNumber, std::chrono::seconds elapsed)
{
    StackTrace trace;

    // Taken before the file is opened: the less that happens between stopping
    // the watched thread and starting it again, the better.
    const bool captured = captureWatchedThreadStackTrace(trace);

    NativeChar path[maxReportPath]{};

    if (! buildReportPath(path, maxReportPath, "hang", "txt"))
    {
        return;
    }

    ReportWriter writer(path);

    writer.write("=== Machines hang report ===");
    writer.newLine();

    writer.write("The main loop has not reported progress for ");
    writer.writeSigned(elapsed.count());
    writer.write(" seconds.");
    writer.newLine();

    writer.write("This is report ");
    writer.writeSigned(reportNumber);
    writer.write(" of at most ");
    writer.writeSigned(maxReports);
    writer.write(". Compare their call stacks: one that does not change is a");
    writer.newLine();
    writer.write("deadlock, one that does is an operation that is merely slow.");
    writer.newLine();

    writer.write("Process: ");
    writer.writeUnsigned(currentProcessId());
    writer.newLine();

    writeBuildInfo(writer);
    writeModuleInfo(writer);

    writer.newLine();
    writer.write("--- Call stack of the main thread ---");
    writer.newLine();

    if (captured)
    {
        writeStackTrace(writer, trace);
    }
    else
    {
        writer.write("  (the main thread could not be stopped to read it)");
        writer.newLine();
    }

    writer.newLine();
    writer.write("=== End of report ===");
    writer.newLine();

    if (writer.isOpen())
    {
        announceReport(path);
    }
}

void watch()
{
    using Clock = std::chrono::steady_clock;

    unsigned long long lastSeen = beats_.load(std::memory_order_relaxed);
    Clock::time_point lastProgress = Clock::now();
    int reportsWritten{};

    // Often enough that the deadline is not overshot by much, rarely enough to
    // cost nothing measurable.
    const std::chrono::milliseconds interval = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_) / 8;

    for (;;)
    {
        {
            std::unique_lock<std::mutex> lock(stopMutex_);

            // A plain sleep here would hold the shutdown up for whatever is
            // left of the interval, which is most of a second at the default
            // timeout and is the last thing the player sees.
            if (stopRequested_.wait_for(lock, interval, [] { return ! watching_; }))
            {
                return;
            }
        }

        const unsigned long long beats = beats_.load(std::memory_order_relaxed);

        if (beats != lastSeen)
        {
            lastSeen = beats;
            lastProgress = Clock::now();
            reportsWritten = 0;
            continue;
        }

        const auto stalled = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - lastProgress);

        if (stalled < timeout_ || reportsWritten >= maxReports)
        {
            continue;
        }

        // Every single step under a debugger looks exactly like a hang from
        // here, and a person who is already looking at the stack does not need
        // this to tell them what it is.
        if (debuggerAttached())
        {
            lastProgress = Clock::now();
            continue;
        }

        ++reportsWritten;
        writeHangReport(reportsWritten, stalled);

        // Report again only after the same wait, so that successive reports are
        // spaced widely enough for a moving stack to have moved.
        lastProgress = Clock::now();
    }
}

} // namespace

void heartbeat()
{
    beats_.fetch_add(1, std::memory_order_relaxed);
}

void startWatchdog(std::chrono::seconds timeout)
{
    if (timeout.count() <= 0)
    {
        return;
    }

    // Held until the end, so that the watcher cannot look at any of this before
    // all of it is set.
    std::lock_guard<std::mutex> lock(stopMutex_);

    if (watching_)
    {
        return;
    }

    timeout_ = timeout;

    rememberWatchedThread();

    watching_ = true;
    watcher_ = std::thread(watch);
}

void stopWatchdog()
{
    {
        std::lock_guard<std::mutex> lock(stopMutex_);

        if (!watching_)
        {
            return;
        }

        watching_ = false;
    }

    stopRequested_.notify_all();

    if (watcher_.joinable())
    {
        watcher_.join();
    }
}

} // namespace CrashDump
