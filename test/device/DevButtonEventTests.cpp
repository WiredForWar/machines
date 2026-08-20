#include <gtest/gtest.h>

#include "device/ButtonEvent.hpp"
#include "device/Time.hpp"

namespace
{

DevButtonEvent makeEvent(
    DevButtonEvent::ScanCode code,
    DevButtonEvent::Action action,
    double time = 10000.0,
    char print = 0)
{
    return DevButtonEvent{ code, action, false, false, false, false, time, 20, 20, 1, print };
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DevButtonEventTests, AgeIsMeasuredBackFromTheEventTime)
{
    // The device clock runs from process start, so a time one second behind it
    // is an event that is one second old.
    const double oneSecondAgo = DevTime::instance().time() - 1.0;

    const DevButtonEvent buttonEvent
        = makeEvent(DevButtonEvent::ScanCode::HOME_PAD, DevButtonEvent::PRESS, oneSecondAgo, 'H');

    // The event holds its time as a float and the clock keeps running, so this
    // is a neighbourhood rather than an equality.
    ASSERT_NEAR(1.0, buttonEvent.age(), 0.5);
}

TEST(DevButtonEventTests, GetPrintableCharOfButtonEvent_WhenInvalid)
{
    const DevButtonEvent buttonEvent
        = makeEvent(DevButtonEvent::ScanCode::HOME_PAD, DevButtonEvent::PRESS, 10000.0, '\xF4');

    // In Machines and PlanetEd, the PRE() would flip out. Not here :)
    ASSERT_EQ('\xF4', buttonEvent.printableChar());
    ASSERT_FALSE(buttonEvent.isPrintable());
}

TEST(DevButtonEventTests, GetActionOfButtonEvent)
{
    using ScanCode = DevButtonEvent::ScanCode;

    ASSERT_EQ(DevButtonEvent::PRESS, makeEvent(ScanCode::MOUSE_MIDDLE, DevButtonEvent::PRESS).action());
    ASSERT_EQ(DevButtonEvent::RELEASE, makeEvent(ScanCode::MOUSE_MIDDLE, DevButtonEvent::RELEASE).action());
    ASSERT_EQ(DevButtonEvent::SCROLL_UP, makeEvent(ScanCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP).action());
    ASSERT_EQ(DevButtonEvent::SCROLL_DOWN, makeEvent(ScanCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN).action());
}
