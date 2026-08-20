#include "device/Input.hpp"

#include "device/Mouse.hpp"
#include "device/private/SdlKeyboard.hpp"

void Device::submitButtonEvent(const DevButtonEvent& event)
{
    if (isMouseButton(event.scanCode()))
    {
        DevMouse::instance().submitEvent(event);
    }
    else
    {
        DevSdlKeyboard::sdlInstance().submitKeyEvent(event);
    }
}

void Device::submitCharEvent(const DevButtonEvent& event)
{
    DevSdlKeyboard::sdlInstance().submitCharEvent(event);
}

void Device::submitPointerPosition(int x, int y)
{
    DevMouse::instance().position(x, y);
}

void Device::submitPointerMotion(double relativeX, double relativeY)
{
    DevMouse::instance().addRelativeMotion(relativeX, relativeY);
}

void Device::submitFocusLost()
{
    DevSdlKeyboard::sdlInstance().submitFocusLost();
    DevMouse::instance().submitFocusLost();
}
