#include "device/Mouse.hpp"

// My eyes can't do it
#define DEV_MOUSE_CLASS DevMouseT<RecRecorderDep, RecRecorderPrivDep, DevTimeDep, DEQDep>

// static
template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
DEV_MOUSE_CLASS& DEV_MOUSE_CLASS::instance()
{
    static DevMouse instance_;
    return instance_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
DEV_MOUSE_CLASS::DevMouseT()
    : sdlDelegate_()
    , pSdl_(&sdlDelegate_)
    , position_(0, 0)
    , lastPosition_(0, 0)
    , relativeMotion_()
    , lButtonPressed_(false)
    , rButtonPressed_(false)
    , scaleX_(1)
    , scaleY_(1)
    , scrolledUp_(false)
    , scrolledDown_(false)
{
    resetPosition();
    lastPosition_ = position_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
DEV_MOUSE_CLASS::DevMouseT(SdlDelegate* useInstead)
    : DevMouseT()
{
    pSdl_ = useInstead;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
DEV_MOUSE_CLASS::~DevMouseT()
{
}

// Windows actually implements the counter which gives the
// nested behaviour specified by the DevMouseT interface.
template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::hide()
{
    pSdl_->showCursor(false);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::unhide()
{
    pSdl_->showCursor(true);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
bool DEV_MOUSE_CLASS::isHidden() const
{
    return cursorVisible_ < 0;
}

extern int mouseSleepTime;

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::position(XCoord new_x, YCoord new_y)
{
    position_.first = static_cast<XCoord>(new_x * scaleX_);
    position_.second = static_cast<YCoord>(new_y * scaleY_);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::changePosition(XCoord new_x, YCoord new_y)
{
    position(new_x / scaleX_, new_y / scaleY_);

    // SDL call to move the mouse cursor to the new position
    // SDL_WarpMouseInWindow
    pSdl_->moveCursorToPosition(nullptr, new_x / scaleX_, new_y / scaleY_);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
const typename DEV_MOUSE_CLASS::Position& // RETURN TYPE. Method below:
DEV_MOUSE_CLASS::position() const
{
    return position_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::addRelativeMotion(double deltaX, double deltaY)
{
    relativeMotion_.x += deltaX;
    relativeMotion_.y += deltaY;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
typename DEV_MOUSE_CLASS::Motion // RETURN TYPE. Method below:
DEV_MOUSE_CLASS::takeRelativeMotion()
{
    const Motion result = relativeMotion_;
    relativeMotion_ = Motion();
    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
const typename DEV_MOUSE_CLASS::Position // RETURN TYPE. Method below:
DEV_MOUSE_CLASS::deltaPosition() const
{
    Position retval = position();
    retval.first -= lastPosition_.first;
    retval.second -= lastPosition_.second;
    return retval;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
typename DEV_MOUSE_CLASS::ButtonState // RETURN TYPE. Method below:
DEV_MOUSE_CLASS::deltaLeftButton() const
{
    bool currentLeftButtonState = leftButton();

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

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
typename DEV_MOUSE_CLASS::ButtonState // RETURN TYPE. Method below:
DEV_MOUSE_CLASS::deltaRightButton() const
{
    bool currentRightButtonState = rightButton();

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

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::wm_button(const DevButtonEventType& ev)
{
    // Decode the message and set this object's internal state.
    switch (ev.scanCode())
    {
        case Device::KeyCode::MOUSE_LEFT:
            lButtonPressed_ = ev.action() == DevButtonEventType::PRESS;
            break;
        case Device::KeyCode::MOUSE_RIGHT:
            rButtonPressed_ = ev.action() == DevButtonEventType::PRESS;
            break;
        case Device::KeyCode::MOUSE_MIDDLE:
            scrolledUp_ = ev.action() == DevButtonEventType::SCROLL_UP;
            scrolledDown_ = ev.action() == DevButtonEventType::SCROLL_DOWN;
            break;
        default:
            break;
    }

    // The coords in the event should already be scaled correctly, so bypass
    // the position set method because it also applies a scale.
    position_.first = ev.cursorCoords().x();
    position_.second = ev.cursorCoords().y();

    // Pass the message onto the event queue.
    eventQueueDependency_.get().queueEvent(ev);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::announceButtonRelease(Device::KeyCode code)
{
    const DevButtonEventType ev(
        code,
        DevButtonEventType::RELEASE,
        true, // previous: the button was down
        false, // shift
        false, // ctrl
        false, // alt
        DevTimeDep::instance().time(),
        position_.first,
        position_.second,
        1); // repeat count must be >= 1
    eventQueueDependency_.get().queueEvent(ev);
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::wm_killfocus()
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

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
bool DEV_MOUSE_CLASS::leftButton() const
{
    return lButtonPressed_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
bool DEV_MOUSE_CLASS::rightButton() const
{
    return rButtonPressed_;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
bool DEV_MOUSE_CLASS::wheelScrollUp() const
{
    const bool result = scrolledUp_;

    // We shall say the mouse is no longer scrolling until the next scroll event flips it back to true
    scrolledUp_ = false;
    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
bool DEV_MOUSE_CLASS::wheelScrollDown() const
{
    const bool result = scrolledDown_;

    // We shall say the mouse is no longer scrolling until the next scroll event flips it back to true
    scrolledDown_ = false;
    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::scaleCoordinates(XCoord xmax, YCoord ymax)
{
    maxPosition_.first = xmax;
    maxPosition_.second = ymax;
    // const double screenX = GetSystemMetrics(SM_CXSCREEN);
    // const double screenY = GetSystemMetrics(SM_CYSCREEN);

    // scaleX_ = xmax / screenX;
    // scaleY_ = ymax / screenY;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
typename DEV_MOUSE_CLASS::Position //  RETURN TYPE. Method below:
DEV_MOUSE_CLASS::getMessagePos() const
{
    // SDL_GetMouseState
    const auto& unscaledXY = pSdl_->getCursorPosition();

    Position result;
    result.first = static_cast<XCoord>(unscaledXY.first * scaleX_);
    result.second = static_cast<YCoord>(unscaledXY.second * scaleY_);

    return result;
}

template <typename RecRecorderDep, typename RecRecorderPrivDep, typename DevTimeDep, typename DEQDep>
void DEV_MOUSE_CLASS::resetPosition()
{
    const XCoord x = (minRange().first + maxRange().first) / 2;
    const YCoord y = (minRange().second + maxRange().second) / 2;
    position(x, y);
}

// Instantiate the template identified by DevMouse alias
template class DevMouseT<RecRecorder, RecRecorderPrivate, DevTime, DevEventQueue>;
