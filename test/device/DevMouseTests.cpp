#include <utility>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "device/Mouse.hpp"
#include "device/SdlMouse.cpp"

using ::testing::TypedEq;
using ::testing::Return;

class MockDevTime
{
public:
    MOCK_METHOD(double, time, (), (const));
};

class MockDevEventQueue
{
public:
    MOCK_METHOD(void, queueEvent, (const DevButtonEvent& event), ());
};

class MockRecRecorder
{
public:
    MOCK_METHOD(RecRecorder::State, state, (), (const));
};

class MockRecRecorderPrivate
{
};

class MockSdlDelegate : public SdlDelegate
{
public:
    MOCK_METHOD(bool, showCursor, (const bool show), (override));
    using IntPair = std::pair<int,int>;
    MOCK_METHOD(IntPair, getCursorPosition, (), (override));
    MOCK_METHOD(void, moveCursorToPosition, (SDL_Window* window, const int x, const int y), (override));
};

class Mouse : public DevMouseT<MockRecRecorder, MockRecRecorderPrivate, MockDevTime, MockDevEventQueue>
{
public:
    explicit Mouse(SdlDelegate* sdl)
        : DevMouseT(sdl)
    {}

    void setMocks(MockRecRecorder* rec, MockRecRecorderPrivate* recPriv, MockDevEventQueue* eq)
    {
        this->recorderDependency_.set(*rec);
        this->recorderPrivDependency_.set(*recPriv);
        this->eventQueueDependency_.set(*eq);
    }

    using DevMouseT<MockRecRecorder, MockRecRecorderPrivate, MockDevTime, MockDevEventQueue>::wm_button;
};

///////////////////////////////////////////////////////////////////////////////

TEST(DevMouseTests, WMbutton_DispatchesClick)
{
    MockSdlDelegate mockSDL;
    MockRecRecorder recorder;
    MockRecRecorderPrivate privRecorder;
    MockDevEventQueue eventQueue;

    // Essentially AfxSdlApp::dispatchMouseEvent()
    const auto code = Device::KeyCode::MOUSE_LEFT;
    const auto act  = DevButtonEvent::PRESS;
    const bool previous = false;
    const bool shift = false;
    const bool ctrl = false;
    const bool alt = false;
    const double time = 1000.0;
    const int x = 100;
    const int y = 100;
    const size_t repeats = 1;
    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);

    EXPECT_CALL(eventQueue, queueEvent(TypedEq<decltype(ev)&>(ev)))
            .Times(1);

    EXPECT_CALL(recorder, state())
            .WillRepeatedly(Return(RecRecorder::INACTIVE));

    Mouse mouse(&mockSDL);
    mouse.setMocks(&recorder, &privRecorder, &eventQueue);

    // DISPATCH THE MOUSE EVENT
    mouse.wm_button(ev);

    const auto& position = mouse.position();
    ASSERT_TRUE(mouse.leftButton());
    ASSERT_EQ(x, position.first);
    ASSERT_EQ(y, position.second);
}

TEST(DevMouseTests, WMbutton_DispatchesScrollUp)
{
    MockSdlDelegate mockSDL;
    MockRecRecorder recorder;
    MockRecRecorderPrivate privRecorder;
    MockDevEventQueue eventQueue;

    // Essentially AfxSdlApp::dispatchMouseScrollEvent()
    const auto code = Device::KeyCode::MOUSE_MIDDLE;
    const auto act  = DevButtonEvent::SCROLL_UP;
    const bool previous = false;
    const bool shift = false;
    const bool ctrl = false;
    const bool alt = false;
    const double time = 1000.0;
    const int x = 100;
    const int y = 100;
    const size_t repeats = 1;
    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);

    EXPECT_CALL(eventQueue, queueEvent(TypedEq<decltype(ev)&>(ev)))
            .Times(1);

    EXPECT_CALL(recorder, state())
            .WillRepeatedly(Return(RecRecorder::INACTIVE));

    Mouse mouse(&mockSDL);
    mouse.setMocks(&recorder, &privRecorder, &eventQueue);

    // DISPATCH THE MOUSE EVENT
    mouse.wm_button(ev);

    const auto& position = mouse.position();
    ASSERT_FALSE(mouse.leftButton());
    ASSERT_FALSE(mouse.rightButton());
    ASSERT_EQ(x, position.first);
    ASSERT_EQ(y, position.second);
    ASSERT_TRUE(mouse.wheelScrollUp());
    // Unless mouse.wm_button(ev) is called again with another scroll, this shall be false.
    ASSERT_FALSE(mouse.wheelScrollUp());
}

TEST(DevMouseTests, WMbutton_DispatchesScrollDown)
{
    MockSdlDelegate mockSDL;
    MockRecRecorder recorder;
    MockRecRecorderPrivate privRecorder;
    MockDevEventQueue eventQueue;

    // Essentially AfxSdlApp::dispatchMouseScrollEvent()
    const auto code = Device::KeyCode::MOUSE_MIDDLE;
    const auto act  = DevButtonEvent::SCROLL_DOWN;
    const bool previous = false;
    const bool shift = false;
    const bool ctrl = false;
    const bool alt = false;
    const double time = 1000.0;
    const int x = 100;
    const int y = 100;
    const size_t repeats = 1;
    const DevButtonEvent ev(code, act, previous, shift, ctrl, alt, time, x, y, repeats);

    EXPECT_CALL(eventQueue, queueEvent(TypedEq<decltype(ev)&>(ev)))
            .Times(1);

    EXPECT_CALL(recorder, state())
            .WillRepeatedly(Return(RecRecorder::INACTIVE));

    Mouse mouse(&mockSDL);
    mouse.setMocks(&recorder, &privRecorder, &eventQueue);

    // DISPATCH THE MOUSE EVENT
    mouse.wm_button(ev);

    const auto& position = mouse.position();
    ASSERT_FALSE(mouse.leftButton());
    ASSERT_FALSE(mouse.rightButton());
    ASSERT_EQ(x, position.first);
    ASSERT_EQ(y, position.second);
    ASSERT_TRUE(mouse.wheelScrollDown());
    // Unless mouse.wm_button(ev) is called again with another scroll, this shall be false.
    ASSERT_FALSE(mouse.wheelScrollDown());
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

    const auto motion = mouse.takeRelativeMotion();
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
    MockRecRecorder recorder;
    MockRecRecorderPrivate privRecorder;
    MockDevEventQueue eventQueue;

    EXPECT_CALL(recorder, state())
            .WillRepeatedly(Return(RecRecorder::INACTIVE));

    Mouse mouse(&mockSDL);
    mouse.setMocks(&recorder, &privRecorder, &eventQueue);

    // A pointer the system holds in place - pinned at a screen edge, or locked for
    // relative reporting - travels without its position ever changing.
    mouse.position(100, 100);
    mouse.addRelativeMotion(50.0, -50.0);

    const auto& position = mouse.position();
    ASSERT_EQ(100, position.first);
    ASSERT_EQ(100, position.second);

    const auto motion = mouse.takeRelativeMotion();
    ASSERT_DOUBLE_EQ(50.0, motion.x);
    ASSERT_DOUBLE_EQ(-50.0, motion.y);
}
