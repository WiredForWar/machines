#include <gtest/gtest.h>

#include "device/EventQueue.hpp"
#include "device/Input.hpp"
#include "device/InputState.hpp"
#include "device/Keyboard.hpp"
#include "device/Mouse.hpp"
#include "device/Time.hpp"

namespace
{

DevButtonEvent makeEvent(Device::KeyCode code, DevButtonEvent::Action action, int x = 0, int y = 0)
{
    return DevButtonEvent{ code, action, false, KeyModifierFlags(), DevTime::instance().time(), x, y, 1 };
}

// Put the process-wide input state and queue back to a known point: ask for the
// codes these tests use, and clear what an earlier test left held or queued.
DevEventQueue& resetSharedInput()
{
    Device::InputState::instance().releaseAllButtons();
    Device::InputState::instance().takePointerMotion();

    DevEventQueue& queue = DevEventQueue::instance();

    queue.queueEvents(Device::KeyCode::MOUSE_LEFT);
    queue.queueEvents(Device::KeyCode::MOUSE_RIGHT);
    queue.queueEvents(Device::KeyCode::KEY_W);
    queue.queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);

    while (!queue.isEmpty())
        queue.oldestEvent();

    return queue;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DevInputTests, AMouseCodeReachesTheMouse)
{
    resetSharedInput();

    Device::submitButtonEvent(makeEvent(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::PRESS, 40, 50));

    ASSERT_TRUE(DevMouse::instance().leftButton());
    ASSERT_EQ(40, DevMouse::instance().position().x);
    ASSERT_EQ(50, DevMouse::instance().position().y);

    Device::submitButtonEvent(makeEvent(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::RELEASE, 40, 50));
    ASSERT_FALSE(DevMouse::instance().leftButton());
}

TEST(DevInputTests, AKeyCodeReachesTheKeyboard)
{
    resetSharedInput();

    Device::submitButtonEvent(makeEvent(Device::KeyCode::KEY_W, DevButtonEvent::PRESS));

    ASSERT_TRUE(DevKeyboard::instance().keyCode(Device::KeyCode::KEY_W));
    ASSERT_TRUE(DevKeyboard::instance().anyKey());

    Device::submitButtonEvent(makeEvent(Device::KeyCode::KEY_W, DevButtonEvent::RELEASE));

    ASSERT_FALSE(DevKeyboard::instance().keyCode(Device::KeyCode::KEY_W));
}

TEST(DevInputTests, ASubmittedEventReachesTheQueueUnchanged)
{
    DevEventQueue& queue = resetSharedInput();

    const DevButtonEvent submitted = makeEvent(Device::KeyCode::KEY_W, DevButtonEvent::PRESS, 7, 9);
    Device::submitButtonEvent(submitted);

    ASSERT_EQ(1, queue.length());
    const DevButtonEvent delivered = queue.oldestEvent();
    ASSERT_TRUE(submitted == delivered);
}

TEST(DevInputTests, PointerPositionAndTravelAreSeparate)
{
    Device::submitPointerPosition(200, 300);
    Device::submitPointerMotion(4.5, -1.5);

    // A captured pointer travels without moving, so the two must not be
    // derived from each other.
    ASSERT_EQ(200, DevMouse::instance().position().x);
    ASSERT_EQ(300, DevMouse::instance().position().y);

    const DevMouse::Motion travel = DevMouse::instance().takeRelativeMotion();
    ASSERT_DOUBLE_EQ(4.5, travel.x);
    ASSERT_DOUBLE_EQ(-1.5, travel.y);
}

TEST(DevInputTests, FocusLossReleasesBothDevices)
{
    DevEventQueue& queue = resetSharedInput();

    Device::submitButtonEvent(makeEvent(Device::KeyCode::MOUSE_RIGHT, DevButtonEvent::PRESS));
    Device::submitButtonEvent(makeEvent(Device::KeyCode::KEY_W, DevButtonEvent::PRESS));
    ASSERT_TRUE(DevMouse::instance().rightButton());
    ASSERT_TRUE(DevKeyboard::instance().keyCode(Device::KeyCode::KEY_W));

    Device::submitFocusLost();

    ASSERT_FALSE(DevMouse::instance().rightButton());
    ASSERT_FALSE(DevKeyboard::instance().keyCode(Device::KeyCode::KEY_W));
    ASSERT_FALSE(DevKeyboard::instance().anyKey());

    // Two presses and a release for each of them.
    ASSERT_EQ(4, queue.length());
}
