#pragma once

#include "device/key.hpp"

class KeyModifierFlags
{
public:
    using Underlying = uint16_t;
    static constexpr Underlying Alt = static_cast<Underlying>(Device::KeyModifier::Alt);
    static constexpr Underlying Ctrl = static_cast<Underlying>(Device::KeyModifier::Ctrl);
    static constexpr Underlying Shift = static_cast<Underlying>(Device::KeyModifier::Shift);
    static constexpr Underlying All = Alt|Ctrl|Shift;

    static constexpr Device::KeyModifier Order[] = {
        Device::KeyModifier::Ctrl,
        Device::KeyModifier::Alt,
        Device::KeyModifier::Shift,
    };

    KeyModifierFlags() = default;
    KeyModifierFlags(Device::KeyModifier modifier);
    explicit KeyModifierFlags(Underlying v);

    static KeyModifierFlags fromCtrlAltShiftState(bool ctrl, bool alt, bool shift);

    bool operator==(const KeyModifierFlags&) const = default;

    KeyModifierFlags operator|(Device::KeyModifier modifier) const noexcept;
    KeyModifierFlags operator&(const KeyModifierFlags flags) const noexcept;

    operator bool() const;

    Underlying value {};
};

inline KeyModifierFlags::KeyModifierFlags(Device::KeyModifier modifier)
    : value(static_cast<Underlying>(modifier))
{
}

inline KeyModifierFlags::KeyModifierFlags(Underlying v)
    : value(v)
{
}

inline KeyModifierFlags KeyModifierFlags::fromCtrlAltShiftState(bool ctrl, bool alt, bool shift)
{
    return KeyModifierFlags(Ctrl * ctrl | Alt * alt | Shift * shift);
}

inline KeyModifierFlags KeyModifierFlags::operator|(Device::KeyModifier modifier) const noexcept
{
    return Device::KeyModifier(value | static_cast<Underlying>(modifier));
}

inline KeyModifierFlags KeyModifierFlags::operator&(const KeyModifierFlags flags) const noexcept
{
    return KeyModifierFlags(value & flags.value);
}

inline KeyModifierFlags::operator bool() const
{
    return value != 0;
}

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
    bool operator==(Underlying v) const
    {
        return value == v;
    }

    KeyWithModifiers(Device::KeyCode code, KeyModifierFlags modFlags = {})
        : value(static_cast<Underlying>(code) | modFlags.value)
    {
    }

    Device::KeyCode keyCode() const noexcept
    {
        return Device::getKeyWithoutModifiers(value);
    }

    KeyModifierFlags modifiers() const noexcept
    {
        return static_cast<Device::KeyModifier>(value & KeyModifierFlags::All);
    }

    KeyWithModifiers operator|(KeyModifierFlags flags) const noexcept
    {
        return KeyWithModifiers(value | flags.value);
    }

    Underlying operator&(const KeyModifierFlags flags) const noexcept
    {
        return value & flags.value;
    }

    Underlying operator&(const KeyWithModifiers keyWithMods) const noexcept
    {
        return value & keyWithMods.value;
    }

private:
    explicit KeyWithModifiers(Underlying v)
        : value(v)
    {
    }
    Underlying value {};
};

inline KeyWithModifiers operator|(Device::KeyCode code, Device::KeyModifier mod)
{
    return KeyWithModifiers(code, mod);
}

inline KeyWithModifiers operator|(Device::KeyCode code, KeyModifierFlags mods)
{
    return KeyWithModifiers(code, mods);
}

inline bool operator==(KeyWithModifiers::Underlying u, const KeyWithModifiers keyWithMods)
{
    return keyWithMods == u;
}
