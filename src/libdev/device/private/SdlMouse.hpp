#pragma once

#include "device/ButtonEvent.hpp"
#include "device/SdlDelegate.hpp"
#include "device/private/SharedMouse.hpp"

#include "base/base.hpp"

// Mouse support under SDL. This class should share the same protocol as other
// classes for different OSs.
class DevMouse : public DevSharedMouse
{
public:
    static DevMouse& instance();

    //  Return the absolute mouse position
    //  ( clipped by the range limits )
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

    // These all return the change in state since the last time
    // the method was called.
    const Position deltaPosition() const;
    ButtonState deltaLeftButton() const;
    ButtonState deltaRightButton() const;

    // Logically hide/show cursor ( successive calls are nested )
    void hide();
    void unhide();
    bool isHidden() const;

    // Scale the output coordinates to be in the range [0,xmax) and [0,ymax).
    void scaleCoordinates(XCoord xmax, YCoord ymax);

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

    // Where the system reports the pointer to be, in the coordinate system of this
    // class's position fn.
    Position getMessagePos() const;

protected:
    //  Singleton
    DevMouse();
    explicit DevMouse(SdlDelegate* useInstead);
    ~DevMouse();

    void wm_button(const DevButtonEvent&);

    // The window has lost input focus. Any button held at this moment counts as
    // released, and no further event will report it.
    void wm_killfocus();

private:
    friend class AfxSdlApp;

    // Queue a release for a button that will get no release of its own.
    void announceButtonRelease(Device::KeyCode code);

    void resetPosition();

    SdlDelegate sdlDelegate_{};
    SdlDelegate* pSdl_{ &sdlDelegate_ }; // <-- Use me

    mutable Position position_{};
    Position lastPosition_{};
    Motion relativeMotion_{};
    int cursorVisible_{};
    bool lButtonPressed_{};
    bool rButtonPressed_{};
    Position maxPosition_{};
    double scaleX_{ 1.0 };
    double scaleY_{ 1.0 };
    mutable bool scrolledUp_{};
    mutable bool scrolledDown_{};
};
