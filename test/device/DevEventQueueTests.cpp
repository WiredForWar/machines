#include <gtest/gtest.h>

#include "device/EventQueue.hpp"

namespace
{

// The filter tables and the enqueue entry point are protected. Reach them.
class EventQueue : public DevEventQueue
{
public:
    using DevEventQueue::getPressFilterFor;
    using DevEventQueue::getReleaseFilterFor;
    using DevEventQueue::getScrollDownFilter;
    using DevEventQueue::getScrollUpFilter;
    using DevEventQueue::queueEvent;
};

DevButtonEvent makeEvent(Device::KeyCode code, DevButtonEvent::Action action, char print)
{
    return DevButtonEvent{ code, action, false, false, false, false, 1.0, 0, 0, 1, print };
}

} // namespace

///////////////////////////////////////////////////////////////////////////////

TEST(DevEventQueueTests, LengthIsEmpty_WhenQueueEmpty)
{
    EventQueue eventQueue;

    ASSERT_EQ(true, eventQueue.isEmpty());
    ASSERT_EQ(0, eventQueue.length());
}

TEST(DevEventQueueTests, QueueEvents_ScanCode)
{
    EventQueue eventQueue;

    eventQueue.queueEvents(Device::KeyCode::HOME_PAD);
    ASSERT_EQ(true, eventQueue.getPressFilterFor(Device::KeyCode::HOME_PAD));
    ASSERT_EQ(true, eventQueue.getReleaseFilterFor(Device::KeyCode::HOME_PAD));
}

TEST(DevEventQueueTests, DontQueueEvents_ScanCode)
{
    EventQueue eventQueue;

    eventQueue.dontQueueEvents(Device::KeyCode::HOME_PAD);
    ASSERT_EQ(false, eventQueue.getPressFilterFor(Device::KeyCode::HOME_PAD));
    ASSERT_EQ(false, eventQueue.getReleaseFilterFor(Device::KeyCode::HOME_PAD));
}

TEST(DevEventQueueTests, QueueEvents_ScanCodeAndAction)
{
    EventQueue eventQueue;
    auto swapAction = [](EventQueue::Action action) -> EventQueue::Action {
        return (action == EventQueue::Action::PRESS) ? EventQueue::Action::RELEASE : EventQueue::Action::PRESS;
    };

    auto action = EventQueue::Action::PRESS;
    bool wantPress = true;
    bool wantRelease = false;
    for (int c = 1; c < Device::MAX_CODE; ++c)
    {
        eventQueue.queueEvents(static_cast<EventQueue::ScanCode>(c), action);

        ASSERT_EQ(wantPress, eventQueue.getPressFilterFor(static_cast<EventQueue::ScanCode>(c)));
        ASSERT_EQ(wantRelease, eventQueue.getReleaseFilterFor(static_cast<EventQueue::ScanCode>(c)));

        action = swapAction(action);
        wantPress = !wantPress;
        wantRelease = !wantRelease;
    }
}

TEST(DevEventQueueTests, DontQueueEvents_ScanCodeAndAction)
{
    EventQueue eventQueue;
    auto swapAction = [](EventQueue::Action action) -> EventQueue::Action {
        return (action == EventQueue::Action::PRESS) ? EventQueue::Action::RELEASE : EventQueue::Action::PRESS;
    };

    auto action = EventQueue::Action::PRESS;
    for (int c = 1; c < Device::MAX_CODE; ++c)
    {
        eventQueue.dontQueueEvents(static_cast<EventQueue::ScanCode>(c), action);

        ASSERT_EQ(false, eventQueue.getPressFilterFor(static_cast<EventQueue::ScanCode>(c)));
        ASSERT_EQ(false, eventQueue.getReleaseFilterFor(static_cast<EventQueue::ScanCode>(c)));

        action = swapAction(action);
    }
}

TEST(DevEventQueueTests, AnUnrequestedCodeNeverReachesTheQueue)
{
    EventQueue eventQueue;

    eventQueue.queueEvent(makeEvent(Device::KeyCode::HOME_PAD, DevButtonEvent::PRESS, 'H'));

    ASSERT_TRUE(eventQueue.isEmpty());
}

TEST(DevEventQueueTests, QueueNewEventAndRetrieveIt_SingleEvent)
{
    EventQueue eventQueue;

    // You can always count on your HomePad ;p
    const DevButtonEvent homePadEvent = makeEvent(Device::KeyCode::HOME_PAD, DevButtonEvent::PRESS, 'H');

    // Tell the DEQ to filter this key
    eventQueue.queueEvents(Device::KeyCode::HOME_PAD);
    ASSERT_EQ(true, eventQueue.getPressFilterFor(Device::KeyCode::HOME_PAD));

    // filterEvent() is covered by this call
    eventQueue.queueEvent(homePadEvent);
    const DevButtonEvent retrievedEvent = eventQueue.oldestEvent();
    ASSERT_EQ('H', retrievedEvent.getChar());
    // The enqueued event shall always have a repeat count >= 1
    ASSERT_GE(homePadEvent.repeatCount(), retrievedEvent.repeatCount());
    ASSERT_TRUE(eventQueue.isEmpty());
}

TEST(DevEventQueueTests, QueueNewEventAndRetrieveIt_RepeatEvents)
{
    EventQueue eventQueue;

    // You can always count on your HomePad ;p
    const DevButtonEvent homePadEvent = makeEvent(Device::KeyCode::HOME_PAD, DevButtonEvent::PRESS, 'H');

    // Tell the DEQ to filter this key
    eventQueue.queueEvents(Device::KeyCode::HOME_PAD);
    ASSERT_EQ(true, eventQueue.getPressFilterFor(Device::KeyCode::HOME_PAD));

    // Repeated mashings!
    eventQueue.queueEvent(homePadEvent);
    eventQueue.queueEvent(homePadEvent);
    eventQueue.queueEvent(homePadEvent);
    // The events will be compressed
    ASSERT_EQ(1, eventQueue.length());

    // ...and handed back one at a time, each with a repeat count of one.
    for (int i = 0; i != 3; ++i)
    {
        const DevButtonEvent retrievedEvent = eventQueue.oldestEvent();
        ASSERT_EQ('H', retrievedEvent.getChar());
        ASSERT_EQ(1, retrievedEvent.repeatCount());
    }

    ASSERT_TRUE(eventQueue.isEmpty());
}

TEST(DevEventQueueTests, QueueNewEventAndRetrieveIt_MouseScrollEvent)
{
    EventQueue eventQueue;

    const DevButtonEvent scrollUpEvent
        = makeEvent(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP, '\xBE');
    const DevButtonEvent scrollDownEvent
        = makeEvent(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN, '\xBF');

    eventQueue.queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);
    ASSERT_EQ(true, eventQueue.getScrollUpFilter());
    eventQueue.queueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN);
    ASSERT_EQ(true, eventQueue.getScrollDownFilter());

    eventQueue.queueEvent(scrollUpEvent);
    eventQueue.queueEvent(scrollDownEvent);
    ASSERT_EQ(2, eventQueue.length());

    const DevButtonEvent upEvent = eventQueue.oldestEvent();
    const DevButtonEvent downEvent = eventQueue.oldestEvent();
    ASSERT_EQ('\xBE', upEvent.getChar());
    ASSERT_EQ('\xBF', downEvent.getChar());

    eventQueue.dontQueueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_UP);
    ASSERT_EQ(false, eventQueue.getScrollUpFilter());
    eventQueue.dontQueueEvents(Device::KeyCode::MOUSE_MIDDLE, DevButtonEvent::SCROLL_DOWN);
    ASSERT_EQ(false, eventQueue.getScrollDownFilter());
}
