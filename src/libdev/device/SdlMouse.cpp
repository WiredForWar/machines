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
    lastPosition_ = position_;
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

bool DevMouse::isHidden() const
{
    return cursorVisible_ < 0;
}

void DevMouse::position(XCoord new_x, YCoord new_y)
{
    position_.first = static_cast<XCoord>(new_x * scaleX_);
    position_.second = static_cast<YCoord>(new_y * scaleY_);
}

void DevMouse::changePosition(XCoord new_x, YCoord new_y)
{
    position(new_x / scaleX_, new_y / scaleY_);

    // SDL call to move the mouse cursor to the new position
    // SDL_WarpMouseInWindow
    pSdl_->moveCursorToPosition(nullptr, new_x / scaleX_, new_y / scaleY_);
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

const DevMouse::Position DevMouse::deltaPosition() const
{
    Position retval = position();
    retval.first -= lastPosition_.first;
    retval.second -= lastPosition_.second;
    return retval;
}

DevMouse::ButtonState DevMouse::deltaLeftButton() const
{
    const bool currentLeftButtonState = leftButton();

    ButtonState result;

    if (currentLeftButtonState == lastLeftButtonState_)
        result = NO_CHANGE;
    else if (currentLeftButtonState)
        result = PRESSED;
    else
        result = RELEASED;

    lastLeftButtonState_ = currentLeftButtonState;

    return result;
}

DevMouse::ButtonState DevMouse::deltaRightButton() const
{
    const bool currentRightButtonState = rightButton();

    ButtonState result;

    if (currentRightButtonState == lastRightButtonState_)
        result = NO_CHANGE;
    else if (currentRightButtonState)
        result = PRESSED;
    else
        result = RELEASED;

    lastRightButtonState_ = currentRightButtonState;

    return result;
}

void DevMouse::wm_button(const DevButtonEvent& ev)
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

    // The coords in the event should already be scaled correctly, so bypass
    // the position set method because it also applies a scale.
    position_.first = ev.cursorCoords().x();
    position_.second = ev.cursorCoords().y();

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

void DevMouse::wm_killfocus()
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

void DevMouse::scaleCoordinates(XCoord xmax, YCoord ymax)
{
    maxPosition_.first = xmax;
    maxPosition_.second = ymax;
}

DevMouse::Position DevMouse::getMessagePos() const
{
    // SDL_GetMouseState
    const std::pair<int, int>& unscaledXY = pSdl_->getCursorPosition();

    Position result;
    result.first = static_cast<XCoord>(unscaledXY.first * scaleX_);
    result.second = static_cast<YCoord>(unscaledXY.second * scaleY_);

    return result;
}

// Set the position to the middle of the range.
void DevMouse::resetPosition()
{
    const XCoord x = (minRange().first + maxRange().first) / 2;
    const YCoord y = (minRange().second + maxRange().second) / 2;
    position(x, y);
}
