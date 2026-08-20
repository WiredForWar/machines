#pragma once

#include "device/ButtonEvent.hpp"
#include "utility/DependencyProvider.hpp"

#include "base/base.hpp"

#include <deque>

// Stores a FIFO of input device events.
class DevEventQueue
{
public:
    using DevButtonEventType = DevButtonEvent;
    using ScanCode = DevButtonEvent::ScanCode;
    using Action = DevButtonEvent::Action;

    //  Singleton class
    static DevEventQueue& instance();
    virtual ~DevEventQueue();

    // Removes the oldest event from the queue.  If an event has a repeat
    // counts greater than one, this fn. will return multiple copies each of
    // which has its count set to one, hence the post-condition.
    // PRE(!isEmpty());
    // POST(result.repeatCount() == 1);
    DevButtonEvent oldestEvent();

    // For each scan-code you must request that events be queued; otherwise they
    // won't be added to the queue.  You can request both press and release
    // events, using the first form; or you can request just one or the other.
    // By default no events are stored in the queue.
    // PRE(code < Device::KeyCode::MAX_CODE);  applies to all ScanCode args
    void queueEvents(ScanCode);
    void dontQueueEvents(ScanCode);

    // For Scroll Up & Scroll Down for mouse wheel, execute these.
    void queueEvents(ScanCode, Action);
    void dontQueueEvents(ScanCode, Action);

    // Throw away certain events.  Four flavours: absolutely everything; all
    // events relating to a given button; any event older than a given time;
    // and events relating to a given button, older than the given time.
    void discardAll();
    void discard(ScanCode);
    void discardOlderThan(double time);
    void discardOlderThan(double time, ScanCode);

    size_t length() const;
    bool isEmpty() const; // length() == 0

    void CLASS_INVARIANT;

protected:
    DevEventQueue();

    constexpr uchar getReleaseFilterFor(ScanCode code) const { return releaseFilter_[static_cast<int>(code)]; }
    constexpr uchar getPressFilterFor(ScanCode code) const { return pressFilter_[static_cast<int>(code)]; }
    constexpr bool getScrollUpFilter() const { return scrollUpFilter_; }
    constexpr bool getScrollDownFilter() const { return scrollDownFilter_; }

    // PRE(event.scanCode() < Device::KeyCode::MAX_CODE);
    void queueEvent(const DevButtonEvent&);

private:
    void setReleaseFilterFor(ScanCode code, bool value) { releaseFilter_[static_cast<int>(code)] = value; }
    void setPressFilterFor(ScanCode code, bool value) { pressFilter_[static_cast<int>(code)] = value; }

    // Only these classes can add events to the back of the queue.
    friend class DevSdlKeyboard;
    template <typename, typename, typename, typename> friend class DevMouseT;

    // Internal convenience methods.
    bool filterEvent(const DevButtonEvent&) const;

    std::deque<DevButtonEvent> events_{};

    // These tables determine which events are queued.
    uchar releaseFilter_[Device::MAX_CODE]{};
    uchar pressFilter_[Device::MAX_CODE]{};

    bool scrollUpFilter_{};
    bool scrollDownFilter_{};
    bool scrollLeftFilter_{};
    bool scrollRightFilter_{};

    // Operation deliberately revoked
    DevEventQueue(const DevEventQueue&) = delete;
    DevEventQueue& operator=(const DevEventQueue&) = delete;
    bool operator==(const DevEventQueue&) = delete;

public:
    friend inline std::ostream& operator<<(std::ostream& o, const DevEventQueue& t)
    {
        o << "Event queue:" << std::endl;

        for (const DevButtonEvent& event : t.events_)
        {
            o << "\t" << event << std::endl;
        }

        o << std::endl;

        o << "Filter tables:    (press)     (release)\n";

        for (int i = 0; i != Device::MAX_CODE; ++i)
        {
            o << i << "\t\t\t\t   " << static_cast<int>(t.pressFilter_[i]) << "\t\t   "
              << static_cast<int>(t.releaseFilter_[i]) << "\n";
        }

        o << std::endl;

        return o;
    }
};

/* *******************************************************
 * SINGLETON DEPENDENCY PROVIDER
 */
template <> inline DevEventQueue& DependencyProvider<DevEventQueue>::getProvided()
{
    return DevEventQueue::instance();
}
