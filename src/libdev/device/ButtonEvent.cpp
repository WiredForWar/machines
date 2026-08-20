#include "device/ButtonEvent.hpp"

#include "device/Time.hpp"

#include <cctype>

DevButtonEvent::DevButtonEvent(
    ScanCode code,
    Action action,
    bool prev,
    KeyModifierFlags modifiers,
    double time,
    int x,
    int y,
    ushort repeat,
    char print)
    : coords_(MexPoint2d(x, y))
    , code_(code)
    , action_(action)
    , time_(time)
    , repeatCount_(repeat)
    , modifiers_(modifiers)
    , printable_(print)
    , press_(action == PRESS)
    , previous_(prev)
{
}

DevButtonEvent::ScanCode DevButtonEvent::scanCode() const
{
    return code_;
}

DevButtonEvent::Action DevButtonEvent::action() const
{
    return action_;
}

bool DevButtonEvent::previous() const
{
    return previous_;
}

size_t DevButtonEvent::repeatCount() const
{
    return repeatCount_;
}

KeyModifierFlags DevButtonEvent::modifiers() const
{
    return modifiers_;
}

bool DevButtonEvent::wasShiftPressed() const
{
    return modifiers_ & KeyModifierFlags(Device::KeyModifier::Shift);
}

bool DevButtonEvent::wasCtrlPressed() const
{
    return modifiers_ & KeyModifierFlags(Device::KeyModifier::Ctrl);
}

bool DevButtonEvent::wasAltPressed() const
{
    return modifiers_ & KeyModifierFlags(Device::KeyModifier::Alt);
}

double DevButtonEvent::time() const
{
    return time_;
}

double DevButtonEvent::age() const
{
    return DevTime::instance().time() - time_;
}

const DevButtonEvent::Coord& DevButtonEvent::cursorCoords() const
{
    return coords_;
}

char DevButtonEvent::printableChar() const
{
    PRE(isPrintable());
    return printable_;
}

bool DevButtonEvent::isPrintable() const
{
    return std::isprint(static_cast<unsigned char>(printable_)) != 0;
}

char DevButtonEvent::getChar() const
{
    return printable_;
}

bool DevButtonEvent::compressRepeats(const DevButtonEvent& ev)
{
    const bool result = *this == ev;

    if (result)
        repeatCount_ += ev.repeatCount_;

    return result;
}

DevButtonEvent DevButtonEvent::decompressRepeats()
{
    PRE(repeatCount() > 1);

    DevButtonEvent result = *this;
    --repeatCount_;
    result.repeatCount_ = 1;

    POST(result.repeatCount() == 1);
    POST(repeatCount() >= 1);
    return result;
}

bool DevButtonEvent::operator==(const DevButtonEvent& ev) const
{
    return code_ == ev.code_ && action_ == ev.action_ && coords_ == ev.coords_ && printable_ == ev.printable_
        && press_ == ev.press_ && modifiers_ == ev.modifiers_;
}

bool DevButtonEvent::operator<(const DevButtonEvent& ev) const
{
    return code_ < ev.code_;
}

bool DevButtonEvent::isCharEvent() const
{
    return printable_ != 0;
}

bool DevButtonEvent::isKeyEvent() const
{
    // Key event if the printable is zero ( although there is a char 0 it is
    // sufficiently useless as a char event that we can use it to distinguish
    // between key and char events ).
    return printable_ == 0;
}
