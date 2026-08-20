#pragma once

#include "device/Key.hpp"
#include "device/KeyNames.hpp"
#include "device/KeyWithModifiers.hpp"
#include "device/Time.hpp"
#include "mathex/Point2d.hpp"

#include "base/base.hpp"

#include <iomanip>
#include <ostream>

// Describes a button press or release, where a button is anything in
// Device::KeyCode -- a key or a mouse button alike.
class DevButtonEvent
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
    DevButtonEvent() = default;

    // Repeat count NEEDS to be >= 1
    DevButtonEvent(
        ScanCode,
        Action,
        bool previous,
        KeyModifierFlags modifiers,
        double time,
        int x,
        int y,
        ushort repeat,
        char print = 0);

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

    // The modifiers held at the time of the event, which is what a bind is
    // matched against.
    KeyModifierFlags modifiers() const;

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
    bool compressRepeats(const DevButtonEvent& ev);

    // If this event has a repeat count of 2 or more, create a copy whose
    // count is 1 and correspondingly decrement this event's count.
    // PRE(repeatCount() > 1);
    // POST(result.repeatCount() == 1); POST(repeatCount() >= 1);
    DevButtonEvent decompressRepeats();

    // Compares everything *except* the repeat count and the time.
    bool operator==(const DevButtonEvent&) const;

    // This is required by the list template instantiation.  It orders events
    // by their scan code which probably doesn't have a practical use.
    bool operator<(const DevButtonEvent&) const;

private:
    Coord coords_;
    ScanCode code_ = ScanCode::INVALID;
    Action action_ = PRESS;

    // Space is at a premium because these objects are copied by value.  Hence,
    // we use only float precision for time.
    float time_ = 0;
    ushort repeatCount_ = 0;
    KeyModifierFlags modifiers_{};
    char printable_ = 0;
    bool press_ = false;
    bool previous_ = false;
};

inline std::ostream& operator<<(std::ostream& o, const DevButtonEvent& t)
{
    o << "Button event: " << std::setprecision(4) << " age=" << t.age() << " s=" << t.wasShiftPressed()
      << " c=" << t.wasCtrlPressed() << " a=" << t.wasAltPressed() << " rpt=" << t.repeatCount() << " ";
    Device::writeAsString(o, t.scanCode());
    o << ((t.action() == DevButtonEvent::PRESS) ? " down" : " up  ");
    o << " coords=" << t.cursorCoords();
    return o;
}
