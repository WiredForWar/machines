/*
 * B U T E V E N T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _DEVICE_BUTEVENT_HPP
#define _DEVICE_BUTEVENT_HPP

#include <iomanip>
#include "base/base.hpp"
#include "mathex/point2d.hpp"

#include "device/key.hpp"
#include "device/time.hpp"
#include "utility/DependencyProvider.hpp"

// Objects of this type describe a button press or release event.

//  This class must remain bitwise canonical for the recorder
//  functionality to work correctly.
template <typename DevTimeDep = DevTime> class DevButtonEventT
{
public:
    using ScanCode = Device::KeyCode;
    enum Action
    {
        PRESS,
        RELEASE,
        SCROLL_UP,
        SCROLL_DOWN,
        SCROLL_LEFT,
        SCROLL_RIGHT,
    };

    // There's no useful reason for creating default events, but ctl_list
    // appears to require a default ctor.
    DevButtonEventT() = default;

    // Repeat count NEEDS to be >= 1
    DevButtonEventT(
        ScanCode,
        Action,
        bool previous,
        bool shift,
        bool ctrl,
        bool alt,
        double time,
        int x,
        int y,
        ushort repeat,
        char print = 0);

    DevButtonEventT(const DevButtonEventT&) = default;
    DevButtonEventT& operator=(const DevButtonEventT&);

    // True if this DevButtonEvent was created as a char event ( i.e. via a WM_CHAR message ).
    bool isCharEvent() const;

    // True if this DevButtonEvent was created as a key event ( i.e. via a WM_KEYUP or WM_KEYDOWN message ).
    bool isKeyEvent() const;

    // Which button does this event describe?
    ScanCode scanCode() const;
    Action action() const;
    bool previous() const;

    // If this is greater than one, it indicates that the event describes
    // multiple identical events.
    size_t repeatCount() const;

    // The state of the modifier keys at the time of the event.
    bool wasShiftPressed() const;
    bool wasCtrlPressed() const;
    bool wasAltPressed() const;

    // At what time did the event occur?  The return value is in seconds, as
    // returned by DevTime.  Age is now minus the event's time.
    double time() const; // POST(result <= DevTime::instance().time());
    double age() const; // POST(result >= 0);

    // Where was the mouse at the time of the event?
    using Coord = MexPoint2d;
    const Coord& cursorCoords() const;

    // Special support for keys which correspond to printable characters.  If
    // this event corresponds to a non-printable char, then the value is 0.
    // PRE( isPrintable() );
    char printableChar() const;

    // Does char have a printable form ( e.g. 'A' 'B' as opposed to '\0' char zero )
    bool isPrintable() const;

    // Get char regardless of whether it is printable or not.
    char getChar() const;

    // If this event matches the given one, combine the two and increase this
    // one's repeat count.  Returns true if they were combined, then it is
    // assumed that the client won't process the argument event.
    bool compressRepeats(const DevButtonEventT& ev);

    // If this event has a repeat count of 2 or more, create a copy whose
    // count is 1 and correspondingly decrement this event's count.
    // PRE(repeatCount() > 1);
    // POST(result.repeatCount() == 1); POST(repeatCount() >= 1);
    DevButtonEventT decompressRepeats();

    // Compares everything *except* the repeat count and the time.
    bool operator==(const DevButtonEventT&) const;

    // This is required by the list template instantiation.  It orders events
    // by their scan code which probably doesn't have a practical use.
    bool operator<(const DevButtonEventT&) const;

protected:
    DependencyProvider<DevTimeDep> timeDependency_;

private:
    Coord coords_;
    ScanCode code_ = ScanCode::INVALID;
    Action action_ = PRESS;

    // Space is at a premium because these objects are copied by value.  Hence,
    // we use only float precision for time and bitfields for all the bools.
    float time_ = 0;
    ushort repeatCount_ = 0;
    char printable_ = 0;
    bool press_ = false;
    bool previous_ = false;
    bool shift_ = false;
    bool ctrl_ = false;
    bool alt_ = false;
};
template <typename DevTimeDep> std::ostream& operator<<(std::ostream& o, const DevButtonEventT<DevTimeDep>&);

// !!!!!!!! CONCRETE !!!!!!!!!!!!!!!!!!!!!!!!!!!!
using DevButtonEvent = DevButtonEventT<DevTime>;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Avoid ODR hell
template <> inline std::ostream& operator<<(std::ostream& o, const DevButtonEvent& t)
{
    o << "Button event: " << std::setprecision(4) << " age=" << t.age() << " s=" << t.wasShiftPressed()
      << " c=" << t.wasCtrlPressed() << " a=" << t.wasAltPressed() << " rpt=" << t.repeatCount() << " ";
    Device::printScanCode(o, t.scanCode());
    o << ((t.action() == DevButtonEvent::PRESS) ? " down" : " up  ");
    o << " coords=" << t.cursorCoords();
    return o;
}

#endif

/* End BUTEVENT.HPP *************************************************/
