#pragma once

#include "device/ButtonEvent.hpp"

// The way input enters the game. Each call routes to whichever device owns the
// event, so a caller states what happened rather than which object to tell.
namespace Device
{

// A button went down or up, or a wheel turned. The button may be a key or a
// mouse button; both live in KeyCode.
void submitButtonEvent(const DevButtonEvent& event);

// A character was typed, as opposed to the key that produced it.
void submitCharEvent(const DevButtonEvent& event);

// The pointer is now here, in window coordinates. Does not move the on-screen
// pointer.
void submitPointerPosition(int x, int y);

// The pointer travelled this far since the last report, in device counts.
// Independent of its position: a captured pointer travels without moving.
void submitPointerMotion(double relativeX, double relativeY);

// The window lost input focus. Everything held at this moment counts as
// released, and no further event will say so.
void submitFocusLost();

} // namespace Device
