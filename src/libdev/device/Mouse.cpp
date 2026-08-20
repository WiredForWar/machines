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
    Device::InputState::instance().setPointerPosition({ .x = new_x, .y = new_y });
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
    return Device::InputState::instance().pointerPosition();
}

void DevMouse::addRelativeMotion(double deltaX, double deltaY)
{
    Device::InputState::instance().addPointerMotion(deltaX, deltaY);
}

DevMouse::Motion DevMouse::takeRelativeMotion()
{
    return Device::InputState::instance().takePointerMotion();
}

void DevMouse::submitEvent(const DevButtonEvent& ev)
{
    if (ev.action() == DevButtonEvent::PRESS)
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
        KeyModifierFlags(), // nothing is held any more
        DevTime::instance().time(),
        position().x,
        position().y,
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
}

bool DevMouse::leftButton() const
{
    return Device::InputState::instance().isButtonPressed(Device::KeyCode::MOUSE_LEFT);
}

bool DevMouse::rightButton() const
{
    return Device::InputState::instance().isButtonPressed(Device::KeyCode::MOUSE_RIGHT);
}

// Until the system reports where the pointer is, the mouse says the origin.
void DevMouse::resetPosition()
{
    position(0, 0);
}
