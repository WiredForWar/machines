#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "device/EventQueue.hpp"
#include "device/Mouse.hpp"

#include <utility>

class MockSdlDelegate : public SdlDelegate
{
public:
    MOCK_METHOD(bool, showCursor, (const bool show), (override));
    using IntPair = std::pair<int, int>;
    MOCK_METHOD(IntPair, getCursorPosition, (), (override));
    MOCK_METHOD(void, moveCursorToPosition, (SDL_Window * window, const int x, const int y), (override));
};

namespace
{

// A mouse of its own, with the event entry points reachable.
class Mouse : public DevMouse
{
public:
    explicit Mouse(SdlDelegate* sdl)
        : DevMouse(sdl)
    {
    }

    using DevMouse::wm_button;
    using DevMouse::wm_killfocus;
};

DevButtonEvent makeEvent(Device::KeyCode code, DevButtonEvent::Action action, int x = 100, int y = 100)
{
    return DevButtonEvent{ code, action, false, false, false, false, 1000.0, x, y, 1 };
}

// Ask the shared queue for the codes these tests use, and drain what an
// earlier test left behind.
DevEventQueue& freshSharedQueue()
{
    DevEventQueue& queue = DevEventQueue::instance();

    queue.queueEvents(Device::KeyCode::MOUSE_LEFT);
    queue.queueEvents(Device::KeyCode::MOUSE_RIGHT);
    queue.queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);
    queue.queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN);

    while (!queue.isEmpty())
        queue.oldestEvent();

    return queue;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DevMouseTests, WMbutton_DispatchesClick)
{
    DevEventQueue& queue = freshSharedQueue();

    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    // Essentially AfxSdlApp::dispatchMouseButtonEvent()
    const DevButtonEvent ev = makeEvent(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::PRESS);

    mouse.wm_button(ev);

    const DevMouse::Position& position = mouse.position();
    ASSERT_TRUE(mouse.leftButton());
    ASSERT_EQ(100, position.first);
    ASSERT_EQ(100, position.second);

    // The event reached the queue unchanged.
    ASSERT_EQ(1, queue.length());
    ASSERT_TRUE(ev == queue.oldestEvent());
}

TEST(DevMouseTests, WMbutton_DispatchesScrollUp)
{
    freshSharedQueue();

    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    // Essentially AfxSdlApp::dispatchMouseScrollEvent()
    mouse.wm_button(makeEvent(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP));

    const DevMouse::Position& position = mouse.position();
    ASSERT_FALSE(mouse.leftButton());
    ASSERT_FALSE(mouse.rightButton());
    ASSERT_EQ(100, position.first);
    ASSERT_EQ(100, position.second);
    ASSERT_TRUE(mouse.wheelScrollUp());
    // Unless mouse.wm_button(ev) is called again with another scroll, this shall be false.
    ASSERT_FALSE(mouse.wheelScrollUp());
}

TEST(DevMouseTests, WMbutton_DispatchesScrollDown)
{
    freshSharedQueue();

    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    mouse.wm_button(makeEvent(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN));

    const DevMouse::Position& position = mouse.position();
    ASSERT_FALSE(mouse.leftButton());
    ASSERT_FALSE(mouse.rightButton());
    ASSERT_EQ(100, position.first);
    ASSERT_EQ(100, position.second);
    ASSERT_TRUE(mouse.wheelScrollDown());
    // Unless mouse.wm_button(ev) is called again with another scroll, this shall be false.
    ASSERT_FALSE(mouse.wheelScrollDown());
}

TEST(DevMouseTests, FocusLossReleasesAHeldButtonExactlyOnce)
{
    DevEventQueue& queue = freshSharedQueue();

    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    mouse.wm_button(makeEvent(Device::KeyCode::MOUSE_LEFT, DevButtonEvent::PRESS));
    ASSERT_TRUE(mouse.leftButton());
    ASSERT_EQ(1, queue.length());

    mouse.wm_killfocus();

    // Nothing reads as held any more, and the release was announced rather
    // than merely dropped.
    ASSERT_FALSE(mouse.leftButton());
    ASSERT_EQ(2, queue.length());

    queue.oldestEvent();
    const DevButtonEvent release = queue.oldestEvent();
    ASSERT_EQ(Device::KeyCode::MOUSE_LEFT, release.scanCode());
    ASSERT_EQ(DevButtonEvent::RELEASE, release.action());

    // A second focus loss has nothing left to announce.
    mouse.wm_killfocus();
    ASSERT_TRUE(queue.isEmpty());
}

TEST(DevMouseTests, RelativeMotion_StartsAtZero)
{
    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    ASSERT_TRUE(mouse.takeRelativeMotion().isZero());
}

TEST(DevMouseTests, RelativeMotion_AccumulatesUntilTaken)
{
    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    mouse.addRelativeMotion(3.0, -2.0);
    mouse.addRelativeMotion(1.5, -0.5);

    const DevMouse::Motion motion = mouse.takeRelativeMotion();
    ASSERT_DOUBLE_EQ(4.5, motion.x);
    ASSERT_DOUBLE_EQ(-2.5, motion.y);
}

TEST(DevMouseTests, RelativeMotion_IsClearedByTaking)
{
    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    mouse.addRelativeMotion(10.0, 20.0);
    mouse.takeRelativeMotion();

    ASSERT_TRUE(mouse.takeRelativeMotion().isZero());
}

TEST(DevMouseTests, RelativeMotion_IsReportedWhileThePositionStandsStill)
{
    MockSdlDelegate mockSDL;
    Mouse mouse(&mockSDL);

    // A pointer the system holds in place - pinned at a screen edge, or locked for
    // relative reporting - travels without its position ever changing.
    mouse.position(100, 100);
    mouse.addRelativeMotion(50.0, -50.0);

    const DevMouse::Position& position = mouse.position();
    ASSERT_EQ(100, position.first);
    ASSERT_EQ(100, position.second);

    const DevMouse::Motion motion = mouse.takeRelativeMotion();
    ASSERT_DOUBLE_EQ(50.0, motion.x);
    ASSERT_DOUBLE_EQ(-50.0, motion.y);
}
