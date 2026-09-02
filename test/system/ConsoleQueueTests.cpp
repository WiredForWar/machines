#include "system/Console.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

namespace
{

using namespace std::chrono_literals;

// A clock the test moves itself, so that a deadline is reached because the test
// said so and not because the machine was slow.
class TestClock
{
public:
    std::chrono::steady_clock::time_point now() const { return now_; }
    void advance(std::chrono::milliseconds by) { now_ += by; }

private:
    std::chrono::steady_clock::time_point now_{};
};

struct Fixture
{
    TestClock clock{};
    std::vector<std::string> ran{};
    bool finished{};

    System::Console makeConsole()
    {
        System::ConsoleConfig config{};
        config.clock = [this]() { return clock.now(); };
        return System::Console(config);
    }

    // Records that it ran and returns at once.
    void addInstant(System::Console& console, const std::string& name)
    {
        console.registerCommand({ .name = name }, [this, name](const auto&, System::IConsole&) {
            ran.push_back(name);
        });
    }

    // Records that it ran, then says its work carries on until finished is set.
    void addWaiting(System::Console& console, const std::string& name)
    {
        console.registerCommand({ .name = name }, [this, name](const auto&, System::IConsole& target) {
            ran.push_back(name);
            target.waitUntil([this]() { return finished; }, 1000ms, name);
        });
    }
};

} // namespace

TEST(ConsoleQueueTests, ACommandThatDoesNotWaitRunsAtOnce)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addInstant(console, "one");
    fixture.addInstant(console, "two");

    console.submit("one");
    console.submit("two");

    EXPECT_EQ(std::vector<std::string>({ "one", "two" }), fixture.ran);
    EXPECT_FALSE(console.isBusy());
}

TEST(ConsoleQueueTests, AWaitingCommandHoldsTheNextLineBack)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");

    EXPECT_EQ(std::vector<std::string>({ "load" }), fixture.ran);
    EXPECT_TRUE(console.isBusy());
}

TEST(ConsoleQueueTests, TheQueueRunsOnWhenTheWaitIsSatisfied)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");

    // Ticking while the work is unfinished changes nothing.
    console.tick();
    EXPECT_EQ(std::vector<std::string>({ "load" }), fixture.ran);

    fixture.finished = true;
    console.tick();

    EXPECT_EQ(std::vector<std::string>({ "load", "after" }), fixture.ran);
    EXPECT_FALSE(console.isBusy());
}

TEST(ConsoleQueueTests, GivingUpOnAWaitDropsWhatWasQueuedBehindIt)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");

    fixture.clock.advance(1001ms);
    console.tick();

    EXPECT_EQ(std::vector<std::string>({ "load" }), fixture.ran);
    EXPECT_FALSE(console.isBusy());
    EXPECT_FALSE(console.lastError().empty());

    // The dropped line does not turn up on a later tick either.
    console.tick();
    EXPECT_EQ(std::vector<std::string>({ "load" }), fixture.ran);
}

TEST(ConsoleQueueTests, BlockingIsOnToBeginWith)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();

    EXPECT_TRUE(console.blockModeEnabled());
}

TEST(ConsoleQueueTests, WithBlockingOffACommandDoesNotHoldTheInput)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    console.setBlockModeEnabled(false);
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");

    EXPECT_EQ(std::vector<std::string>({ "load", "after" }), fixture.ran);
    EXPECT_FALSE(console.isBusy());
}

TEST(ConsoleQueueTests, CancellingStopsWaitingAndDropsTheQueue)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");
    console.cancelPending();

    EXPECT_FALSE(console.isBusy());

    console.tick();
    EXPECT_EQ(std::vector<std::string>({ "load" }), fixture.ran);
}

TEST(ConsoleQueueTests, WorkAlreadyDoneDoesNotCostAFrame)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.finished = true;
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.submit("load");
    console.submit("after");

    EXPECT_EQ(std::vector<std::string>({ "load", "after" }), fixture.ran);
    EXPECT_FALSE(console.isBusy());
}

TEST(ConsoleQueueTests, AScriptStopsAtItsFirstWaitAndCarriesOnAfterIt)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addInstant(console, "before");
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "after");

    console.executeScript("before\nload\nafter\n");

    EXPECT_EQ(std::vector<std::string>({ "before", "load" }), fixture.ran);

    fixture.finished = true;
    console.tick();

    EXPECT_EQ(std::vector<std::string>({ "before", "load", "after" }), fixture.ran);
}

TEST(ConsoleQueueTests, ALineOfferedWhileBusyKeepsItsPlaceInTheOrder)
{
    Fixture fixture;
    System::Console console = fixture.makeConsole();
    fixture.addWaiting(console, "load");
    fixture.addInstant(console, "first");
    fixture.addInstant(console, "second");

    console.submit("load");
    console.submit("first");
    console.submit("second");

    fixture.finished = true;
    console.tick();

    EXPECT_EQ(std::vector<std::string>({ "load", "first", "second" }), fixture.ran);
}
