#include <gtest/gtest.h>

#include "crashdump/CrashDump.hpp"

#include <chrono>
#include <thread>

namespace
{

// The watcher checks in eight times per timeout, so this leaves it a full second
// between checks: long enough that a stop which waits one out is unmistakable.
constexpr auto timeout = std::chrono::seconds(8);

// Generous against scheduling noise, and still four times inside the interval.
constexpr auto stopBudget = std::chrono::milliseconds(250);

// Stopping the instant it is started proves nothing: the watcher has not reached
// its wait yet and finds the flag already clear whichever way the wait is
// written. The interval only holds a shutdown up once the watcher is inside it.
void startAndLetTheWatcherSettle()
{
    CrashDump::startWatchdog(timeout);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

std::chrono::milliseconds timeStop()
{
    const std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();

    CrashDump::stopWatchdog();

    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - before);
}

} // namespace

// The watcher used to spend the whole interval in an uninterruptible sleep and
// only then notice it had been asked to stop, which put most of a second into
// every shutdown.
TEST(WatchdogTests, StopDoesNotWaitOutTheCheckInterval)
{
    startAndLetTheWatcherSettle();

    EXPECT_LT(timeStop(), stopBudget);
}

TEST(WatchdogTests, StopIsHarmlessWhenNothingIsRunning)
{
    EXPECT_LT(timeStop(), stopBudget);
}

TEST(WatchdogTests, StartsNoThreadForATimeoutOfZero)
{
    CrashDump::startWatchdog(std::chrono::seconds(0));

    EXPECT_LT(timeStop(), stopBudget);
}

TEST(WatchdogTests, CanBeStartedAgainAfterStopping)
{
    startAndLetTheWatcherSettle();
    ASSERT_LT(timeStop(), stopBudget);

    startAndLetTheWatcherSettle();
    EXPECT_LT(timeStop(), stopBudget);
}

// A heartbeat is what the watched thread reports progress with, and it has to
// stay callable whether or not anything is watching.
TEST(WatchdogTests, HeartbeatIsSafeWhileStoppedAndWhileRunning)
{
    CrashDump::heartbeat();

    startAndLetTheWatcherSettle();
    CrashDump::heartbeat();

    EXPECT_LT(timeStop(), stopBudget);
}
