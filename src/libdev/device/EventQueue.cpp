#include "device/EventQueue.hpp"

// static
DevEventQueue& DevEventQueue::instance()
{
    static DevEventQueue instance_;
    return instance_;
}

DevEventQueue::DevEventQueue()
{
    // By default we don't queue anything until asked.
    // The middle mouse wheel is the only button that would receive these.
    // Explicity ask for these using queueEvents w/ middle mouse wheel scan code.

    TEST_INVARIANT;
}

DevEventQueue::~DevEventQueue()
{
    TEST_INVARIANT;
}

DevButtonEvent DevEventQueue::oldestEvent()
{
    PRE(!isEmpty());

    DevButtonEvent result;
    DevButtonEvent& front = events_.front();

    if (front.repeatCount() == 1)
    {
        result = front;
        events_.pop_front();
    }
    else
    {
        result = front.decompressRepeats();
    }

    POST(result.repeatCount() == 1);
    POST(Device::isValidCode(result.scanCode()));
    return result;
}

void DevEventQueue::queueEvent(const DevButtonEvent& event)
{
    PRE(Device::isValidCode(event.scanCode()));

    if (!filterEvent(event))
        return;

    // If there is an event at the back of the queue which matches the
    // current one, they are compressed and we don't add a new event.
    if (!events_.empty() && events_.back().compressRepeats(event))
        return;

    events_.push_back(event);
}

bool DevEventQueue::filterEvent(const DevButtonEvent& event) const
{
    PRE(Device::isValidCode(event.scanCode()));

    switch (event.action())
    {
        case DevButtonEvent::RELEASE:
            return getReleaseFilterFor(event.scanCode());
        case DevButtonEvent::PRESS:
            return getPressFilterFor(event.scanCode());
        case DevButtonEvent::SCROLL_UP:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollUpFilter_);
        case DevButtonEvent::SCROLL_DOWN:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollDownFilter_);
        case DevButtonEvent::SCROLL_LEFT:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollLeftFilter_);
        case DevButtonEvent::SCROLL_RIGHT:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollRightFilter_);
        default:
            ASSERT_BAD_CASE;
            break;
    }

    return false;
}

void DevEventQueue::queueEvents(ScanCode code)
{
    PRE_INFO(static_cast<int>(code));
    PRE(Device::isValidCode(code));
    setReleaseFilterFor(code, true);
    setPressFilterFor(code, true);
}

void DevEventQueue::dontQueueEvents(ScanCode code)
{
    PRE(Device::isValidCode(code));
    setReleaseFilterFor(code, false);
    setPressFilterFor(code, false);
}

void DevEventQueue::queueEvents(ScanCode code, Action action)
{
    PRE(Device::isValidCode(code));

    switch (action)
    {
        case DevButtonEvent::RELEASE:
            setReleaseFilterFor(code, true);
            break;
        case DevButtonEvent::PRESS:
            setPressFilterFor(code, true);
            break;
        case DevButtonEvent::SCROLL_UP:
            // only set for middle mouse
            scrollUpFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case DevButtonEvent::SCROLL_DOWN:
            // only set for middle mouse
            scrollDownFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case DevButtonEvent::SCROLL_LEFT:
            // only set for middle mouse
            scrollLeftFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case DevButtonEvent::SCROLL_RIGHT:
            // only set for middle mouse
            scrollRightFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        default:
            ASSERT_BAD_CASE;
            break;
    }
}

void DevEventQueue::dontQueueEvents(ScanCode code, Action action)
{
    PRE(Device::isValidCode(code));

    switch (action)
    {
        case DevButtonEvent::RELEASE:
            setReleaseFilterFor(code, false);
            break;
        case DevButtonEvent::PRESS:
            setPressFilterFor(code, false);
            break;
        case DevButtonEvent::SCROLL_UP:
            // only set for middle mouse
            scrollUpFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollUpFilter_;
            break;
        case DevButtonEvent::SCROLL_DOWN:
            // only set for middle mouse
            scrollDownFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollDownFilter_;
            break;
        case DevButtonEvent::SCROLL_LEFT:
            // only set for middle mouse
            scrollLeftFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollLeftFilter_;
            break;
        case DevButtonEvent::SCROLL_RIGHT:
            // only set for middle mouse
            scrollRightFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollRightFilter_;
            break;
        default:
            ASSERT_BAD_CASE;
            break;
    }
}

size_t DevEventQueue::length() const
{
    return events_.size();
}

bool DevEventQueue::isEmpty() const
{
    return events_.empty();
}

void DevEventQueue::CLASS_INVARIANT
{
    for (const DevButtonEvent& event : events_)
    {
        INVARIANT(Device::isValidCode(event.scanCode()));
    }
}
