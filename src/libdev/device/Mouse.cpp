#include "device/Mouse.hpp"

#include "device/EventQueue.hpp"
#include "device/Time.hpp"

// static
DevMouse& DevMouse::instance()
{
    static DevMouse instance_;
    return instance_;
}

DevMouse::DevMouse()
{
    resetPosition();
}

DevMouse::DevMouse(SdlDelegate* useInstead)
    : DevMouse()
{
    pSdl_ = useInstead;
}

DevMouse::~DevMouse()
{
}

void DevMouse::hide()
{
    pSdl_->showCursor(false);
}

void DevMouse::unhide()
{
    pSdl_->showCursor(true);
}

void DevMouse::position(XCoord new_x, YCoord new_y)
{
    position_.first = new_x;
    position_.second = new_y;
}

void DevMouse::changePosition(XCoord new_x, YCoord new_y)
{
    position(new_x, new_y);

    // SDL call to move the mouse cursor to the new position
    // SDL_WarpMouseInWindow
    pSdl_->moveCursorToPosition(nullptr, new_x, new_y);
}

const DevMouse::Position& DevMouse::position() const
{
    return position_;
}

void DevMouse::addRelativeMotion(double deltaX, double deltaY)
{
    relativeMotion_.x += deltaX;
    relativeMotion_.y += deltaY;
}

DevMouse::Motion DevMouse::takeRelativeMotion()
{
    const Motion result = relativeMotion_;
    relativeMotion_ = Motion();
    return result;
}

void DevMouse::submitEvent(const DevButtonEvent& ev)
{
    // Decode the message and set this object's internal state.
    switch (ev.scanCode())
    {
        case Device::KeyCode::MOUSE_LEFT:
            lButtonPressed_ = ev.action() == DevButtonEvent::PRESS;
            break;
        case Device::KeyCode::MOUSE_RIGHT:
            rButtonPressed_ = ev.action() == DevButtonEvent::PRESS;
            break;
        case Device::KeyCode::MOUSE_MIDDLE:
            scrolledUp_ = ev.action() == DevButtonEvent::SCROLL_UP;
            scrolledDown_ = ev.action() == DevButtonEvent::SCROLL_DOWN;
            break;
        default:
            break;
    }

    position(ev.cursorCoords().x(), ev.cursorCoords().y());

    // Pass the message onto the event queue.
    DevEventQueue::instance().queueEvent(ev);
}

void DevMouse::announceButtonRelease(Device::KeyCode code)
{
    const DevButtonEvent ev(
        code,
        DevButtonEvent::RELEASE,
        true, // previous: the button was down
        false, // shift
        false, // ctrl
        false, // alt
        DevTime::instance().time(),
        position_.first,
        position_.second,
        1); // repeat count must be >= 1
    DevEventQueue::instance().queueEvent(ev);
}

void DevMouse::submitFocusLost()
{
    // Clear the polled state ahead of the announcement, so that a button never
    // reads as down once its release has been queued.
    if (lButtonPressed_)
    {
        lButtonPressed_ = false;
        announceButtonRelease(Device::KeyCode::MOUSE_LEFT);
    }

    if (rButtonPressed_)
    {
        rButtonPressed_ = false;
        announceButtonRelease(Device::KeyCode::MOUSE_RIGHT);
    }

    scrolledUp_ = false;
    scrolledDown_ = false;
}

bool DevMouse::leftButton() const
{
    return lButtonPressed_;
}

bool DevMouse::rightButton() const
{
    return rButtonPressed_;
}

bool DevMouse::wheelScrollUp() const
{
    const bool result = scrolledUp_;

    // We shall say the mouse is no longer scrolling until the next scroll event flips it back to true
    scrolledUp_ = false;
    return result;
}

bool DevMouse::wheelScrollDown() const
{
    const bool result = scrolledDown_;

    // We shall say the mouse is no longer scrolling until the next scroll event flips it back to true
    scrolledDown_ = false;
    return result;
}

// Until the system reports where the pointer is, the mouse says the origin.
void DevMouse::resetPosition()
{
    position(0, 0);
}
