#pragma once

#include "device/key.hpp"

class KeyModifierFlags
{
public:
    using Underlying = uint16_t;
    static constexpr Underlying Alt = static_cast<Underlying>(Device::KeyModifier::Alt);
    static constexpr Underlying Ctrl = static_cast<Underlying>(Device::KeyModifier::Ctrl);
    static constexpr Underlying Shift = static_cast<Underlying>(Device::KeyModifier::Shift);

    KeyModifierFlags() = default;
    KeyModifierFlags(Device::KeyModifier modifier)
        : value(static_cast<Underlying>(modifier))
    {
    }

    KeyModifierFlags& operator|(Device::KeyModifier modifier)
    {
        value |= static_cast<Underlying>(modifier);
        return *this;
    }

    static KeyModifierFlags fromAltCtrlShiftState(bool alt, bool ctrl, bool shift)
    {
        return KeyModifierFlags(Alt * alt | Ctrl * ctrl | Shift * shift);
    }

    Underlying value {};

private:
    KeyModifierFlags(Underlying v)
        : value(v)
    {
    }
};

inline KeyModifierFlags operator|(Device::KeyModifier mod1, Device::KeyModifier mod2)
{
    return KeyModifierFlags(mod1) | mod2;
}

class KeyWithModifiers
{
public:
    using Underlying = uint16_t;

    KeyWithModifiers() = default;
    KeyWithModifiers(const KeyWithModifiers&) = default;

    auto operator<=>(const KeyWithModifiers&) const = default;

    KeyWithModifiers(Device::KeyCode code, KeyModifierFlags modFlags = {})
        : value(static_cast<Underlying>(code) | modFlags.value)
    {
    }

    KeyWithModifiers& operator|(KeyModifierFlags flags)
    {
        value |= flags.value;
        return *this;
    }

    Underlying value {};
};

inline KeyWithModifiers operator|(Device::KeyCode code, Device::KeyModifier mod)
{
    return KeyWithModifiers(code) | mod;
}

inline KeyWithModifiers operator|(Device::KeyCode code, KeyModifierFlags mods)
{
    return KeyWithModifiers(code) | mods;
}
