/*
 * E V E N T Q . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "device/eventq.hpp"

// static
template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>&
DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::instance()
{
    static DevEventQueue instance_;
    return instance_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevEventQueueT()
    : list_(new ctl_list<typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType>)
{
    ASSERT(list_, "Failed to allocate device event queue.");

    // By default we don't queue anything until asked.
    // The middle mouse wheel is the only button that would receive these.
    // Explicity ask for these using queueEvents w/ middle mouse wheel scan code.

    TEST_INVARIANT;
}

// virtual
template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::~DevEventQueueT()
{
    TEST_INVARIANT;
    delete list_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType
DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::oldestEvent()
{
    using ButtonEvent = typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType;
    ButtonEvent result;

    if (recorderDependency_.get().state() == RecRecorder::PLAYING)
    {
        result = recorderPrivDependency_.get().playbackButtonEvent();
    }
    else
    {
        ButtonEvent& front = list_->front();

        if (front.repeatCount() == 1)
        {
            result = front;
            list_->pop_front();
        }
        else
        {
            result = front.decompressRepeats();
        }

        if (recorderDependency_.get().state() == RecRecorder::RECORDING)
        {
            recorderPrivDependency_.get().recordButtonEvent(result);
        }
    }

    POST(result.repeatCount() == 1);
    POST(Device::isValidCode(result.scanCode()));
    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::queueEvent(
    const typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType& event)
{
    PRE(Device::isValidCode(event.scanCode()));

    if (filterEvent(event))
    {
        // If there is an event at the back of the queue which matches the
        // current one, they are compressed and we don't add a new event.

        //  list_->size() is used rather than length() because this function
        //  is called from the Windows event handler. This stops the recording
        //  working properly. Bob

        if (list_->size() > 0 && list_->back().compressRepeats(event))
        {
            return;
        }
        else
        {
            list_->push_back(event);
        }
    }
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
bool DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::filterEvent(
    const typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType& event) const
{
    using ButtonEvent = typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType;
    PRE(Device::isValidCode(event.scanCode()));

    switch (event.action())
    {
        case ButtonEvent::RELEASE:
            return getReleaseFilterFor(event.scanCode());
        case ButtonEvent::PRESS:
            return getPressFilterFor(event.scanCode());
        case ButtonEvent::SCROLL_UP:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollUpFilter_);
        case ButtonEvent::SCROLL_DOWN:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollDownFilter_);
        case ButtonEvent::SCROLL_LEFT:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollLeftFilter_);
        case ButtonEvent::SCROLL_RIGHT:
            return (event.scanCode() == Device::KeyCode::MOUSE_MIDDLE && scrollRightFilter_);
        default:
            ASSERT_BAD_CASE;
            break;
    }

    return false;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::queueEvents(ScanCode code)
{
    PRE(Device::isValidCode(code));
    setReleaseFilterFor(code, true);
    setPressFilterFor(code, true);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::dontQueueEvents(ScanCode code)
{
    PRE(Device::isValidCode(code));
    setReleaseFilterFor(code, false);
    setPressFilterFor(code, false);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::queueEvents(ScanCode code, Action action)
{
    using ButtonEvent = typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType;
    PRE(Device::isValidCode(code));

    switch (action)
    {
        case ButtonEvent::RELEASE:
            setReleaseFilterFor(code, true);
            break;
        case ButtonEvent::PRESS:
            setPressFilterFor(code, true);
            break;
        case ButtonEvent::SCROLL_UP:
            // only set for middle mouse
            scrollUpFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case ButtonEvent::SCROLL_DOWN:
            // only set for middle mouse
            scrollDownFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case ButtonEvent::SCROLL_LEFT:
            // only set for middle mouse
            scrollLeftFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        case ButtonEvent::SCROLL_RIGHT:
            // only set for middle mouse
            scrollRightFilter_ = code == Device::KeyCode::MOUSE_MIDDLE;
            break;
        default:
            ASSERT_BAD_CASE;
            break;
    }
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::dontQueueEvents(ScanCode code, Action action)
{
    using ButtonEvent = typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType;
    PRE(Device::isValidCode(code));

    switch (action)
    {
        case ButtonEvent::RELEASE:
            setReleaseFilterFor(code, false);
            break;
        case ButtonEvent::PRESS:
            setPressFilterFor(code, false);
            break;
        case ButtonEvent::SCROLL_UP:
            // only set for middle mouse
            scrollUpFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollUpFilter_;
            break;
        case ButtonEvent::SCROLL_DOWN:
            // only set for middle mouse
            scrollDownFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollDownFilter_;
            break;
        case ButtonEvent::SCROLL_LEFT:
            // only set for middle mouse
            scrollLeftFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollLeftFilter_;
            break;
        case ButtonEvent::SCROLL_RIGHT:
            // only set for middle mouse
            scrollRightFilter_ = (code == Device::KeyCode::MOUSE_MIDDLE) ? false : scrollRightFilter_;
            break;
        default:
            ASSERT_BAD_CASE;
            break;
    }
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
size_t DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::length() const
{
    size_t result;

    if (recorderDependency_.get().state() == RecRecorder::PLAYING)
    {
        result = recorderPrivDependency_.get().playbackEventQueueLength();
    }
    else
    {
        result = list_->size();

        if (recorderDependency_.get().state() == RecRecorder::RECORDING)
        {
            recorderPrivDependency_.get().recordEventQueueLength(result);
        }
    }

    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
bool DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::isEmpty() const
{
    return length() == 0;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep>
void DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::CLASS_INVARIANT
{
    INVARIANT(list_);

    // Ayy Lmao
    using ButtonEvent = typename DevEventQueueT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep>::DevButtonEventType;
    using ButtonEvent_Iterator = typename ctl_list<ButtonEvent>::const_iterator;

    // This will fail for default objects!
    ButtonEvent_Iterator it = list_->begin();
    while (it != list_->end())
    {
        const ButtonEvent& ev = *it;
        INVARIANT(Device::isValidCode(ev.scanCode()));
        ++it;
    }
}

// Instantiate the template identified by DevEventQueue alias
template class DevEventQueueT<RecRecorder, RecRecorderPrivate, DevTime>;
/* End EVENTQ.CPP ***************************************************/
