#pragma once

#include "device/ButtonEvent.hpp"
#include "device/SdlDelegate.hpp"

#include "base/base.hpp"

#include <utility>

// Where the pointer is, which of its buttons are held, and the way mouse
// events enter the game.
class DevMouse
{
public:
    using XCoord = int32;
    using YCoord = int32;

    using Position = std::pair<XCoord, YCoord>;

    // Pointer travel in device counts. Fractional because a system may report
    // sub-count travel once its own pointer scaling has been applied.
    struct Motion
    {
        double x = 0.0;
        double y = 0.0;

        bool isZero() const { return x == 0.0 && y == 0.0; }
    };

    enum ButtonState
    {
        NO_CHANGE,
        PRESSED,
        RELEASED
    };

    static DevMouse& instance();

    //  Return the absolute mouse position
    const Position& position() const;

    // Pointer travel reported by the system since the last takeRelativeMotion() call,
    // in device counts, and clear the accumulator. Unlike a difference of two
    // position() values this is not quantised to the window, so it keeps working when
    // the pointer is at an edge of the screen.
    Motion takeRelativeMotion();

    bool leftButton() const;
    bool rightButton() const;
    bool wheelScrollUp() const;
    bool wheelScrollDown() const;

    // Logically hide/show cursor
    void hide();
    void unhide();

    // Set the mouse's position.  Does NOT move the on screen pointer. Called when a
    // pointer motion event is received. It is undesirable to move the on screen pointer
    // as this will generate another motion event.
    void position(XCoord newX, YCoord newY);

    // Set the mouse's position. This function updates the on screen pointer position. Should not be
    // called in response to a motion event because this function generates another motion
    // event.
    void changePosition(XCoord new_x, YCoord new_y);

    // Add pointer travel to the amount reported by takeRelativeMotion(). Called by the
    // application for every pointer motion the system reports.
    void addRelativeMotion(double deltaX, double deltaY);

    // Report a button or wheel event.
    void submitEvent(const DevButtonEvent&);

    // The window has lost input focus. Any button held at this moment counts as
    // released, and no further event will report it.
    void submitFocusLost();

protected:
    //  Singleton
    DevMouse();
    explicit DevMouse(SdlDelegate* useInstead);
    ~DevMouse();

private:
    // Queue a release for a button that will get no release of its own.
    void announceButtonRelease(Device::KeyCode code);

    void resetPosition();

    SdlDelegate sdlDelegate_{};
    SdlDelegate* pSdl_{ &sdlDelegate_ }; // <-- Use me

    Position position_{};
    Motion relativeMotion_{};
    mutable bool scrolledUp_{};
    mutable bool scrolledDown_{};
};
