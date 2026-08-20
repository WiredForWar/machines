#include "device/Mouse.hpp"

#include "device/EventQueue.hpp"
#include "device/InputState.hpp"
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
    if (ev.scanCode() == Device::KeyCode::MOUSE_MIDDLE)
    {
        scrolledUp_ = ev.action() == DevButtonEvent::SCROLL_UP;
        scrolledDown_ = ev.action() == DevButtonEvent::SCROLL_DOWN;
    }
    else if (ev.action() == DevButtonEvent::PRESS)
    {
        Device::InputState::instance().pressButton(ev.scanCode());
    }
    else if (ev.action() == DevButtonEvent::RELEASE)
    {
        Device::InputState::instance().releaseButton(ev.scanCode());
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
    Device::InputState& state = Device::InputState::instance();

    // Clear the polled state ahead of each announcement, so that a button never
    // reads as down once its release has been queued.
    for (const Device::KeyCode code : { Device::KeyCode::MOUSE_LEFT, Device::KeyCode::MOUSE_RIGHT })
    {
        if (!state.isButtonPressed(code))
            continue;

        state.releaseButton(code);
        announceButtonRelease(code);
    }

    scrolledUp_ = false;
    scrolledDown_ = false;
}

bool DevMouse::leftButton() const
{
    return Device::InputState::instance().isButtonPressed(Device::KeyCode::MOUSE_LEFT);
}

bool DevMouse::rightButton() const
{
    return Device::InputState::instance().isButtonPressed(Device::KeyCode::MOUSE_RIGHT);
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
