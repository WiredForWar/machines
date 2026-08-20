#include "device/Input.hpp"

#include "device/Keyboard.hpp"
#include "device/Mouse.hpp"

void Device::submitButtonEvent(const DevButtonEvent& event)
{
    if (isMouseButton(event.scanCode()))
    {
        DevMouse::instance().submitEvent(event);
    }
    else
    {
        DevKeyboard::instance().submitKeyEvent(event);
    }
}

void Device::submitCharEvent(const DevButtonEvent& event)
{
    DevKeyboard::instance().submitCharEvent(event);
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
    DevKeyboard::instance().submitFocusLost();
    DevMouse::instance().submitFocusLost();
}
