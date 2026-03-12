#pragma once

#include "device/Key.hpp"

class KeyModifierFlags
{
public:
    using Underlying = uint16_t;
    static constexpr Underlying Alt = static_cast<Underlying>(Device::KeyModifier::Alt);
    static constexpr Underlying Ctrl = static_cast<Underlying>(Device::KeyModifier::Ctrl);
    static constexpr Underlying Shift = static_cast<Underlying>(Device::KeyModifier::Shift);
    static constexpr Underlying AllMask = static_cast<Underlying>(Alt | Ctrl | Shift);

    static constexpr Device::KeyModifier Order[] = {
        Device::KeyModifier::Ctrl,
        Device::KeyModifier::Alt,
        Device::KeyModifier::Shift,
    };

    KeyModifierFlags() = default;
    constexpr KeyModifierFlags(Device::KeyModifier modifier);
    constexpr explicit KeyModifierFlags(Underlying v);

    static constexpr KeyModifierFlags fromCtrlAltShiftState(bool ctrl, bool alt, bool shift);
    static const KeyModifierFlags All;

    bool operator==(const KeyModifierFlags&) const = default;

    constexpr KeyModifierFlags operator|(Device::KeyModifier modifier) const noexcept;
    constexpr KeyModifierFlags operator&(const KeyModifierFlags flags) const noexcept;

    constexpr operator bool() const noexcept;

    Underlying value {};
};

inline constexpr KeyModifierFlags::KeyModifierFlags(Device::KeyModifier modifier)
    : value(static_cast<Underlying>(modifier))
{
}

inline constexpr KeyModifierFlags::KeyModifierFlags(Underlying v)
    : value(v)
{
}

inline constexpr KeyModifierFlags KeyModifierFlags::fromCtrlAltShiftState(bool ctrl, bool alt, bool shift)
{
    return KeyModifierFlags(Underlying(Ctrl * ctrl | Alt * alt | Shift * shift));
}

inline const KeyModifierFlags KeyModifierFlags::All = KeyModifierFlags(AllMask);

inline constexpr KeyModifierFlags KeyModifierFlags::operator|(Device::KeyModifier modifier) const noexcept
{
    return KeyModifierFlags(Underlying(value | static_cast<Underlying>(modifier)));
}

inline constexpr KeyModifierFlags KeyModifierFlags::operator&(const KeyModifierFlags flags) const noexcept
{
    return KeyModifierFlags(value & flags.value);
}

inline constexpr KeyModifierFlags::operator bool() const noexcept
{
    return value != 0;
}

inline constexpr KeyModifierFlags operator|(Device::KeyModifier mod1, Device::KeyModifier mod2)
{
    return KeyModifierFlags(mod1) | mod2;
}

class KeyWithModifiers
{
public:
    using Underlying = uint16_t;

    KeyWithModifiers() = default;

    auto operator<=>(const KeyWithModifiers&) const = default;
    bool operator==(const KeyWithModifiers&) const = default;
    constexpr bool operator==(Underlying v) const noexcept
    {
        return value == v;
    }

    constexpr KeyWithModifiers(Device::KeyCode code, KeyModifierFlags modFlags = {})
        : value(static_cast<Underlying>(code) | modFlags.value)
    {
    }

    constexpr Device::KeyCode keyCode() const noexcept
    {
        return Device::getKeyWithoutModifiers(value);
    }

    constexpr KeyModifierFlags modifiers() const noexcept
    {
        return KeyModifierFlags(KeyModifierFlags::Underlying(value & KeyModifierFlags::AllMask));
    }

    constexpr KeyWithModifiers operator|(KeyModifierFlags flags) const noexcept
    {
        return KeyWithModifiers(Underlying(value | flags.value));
    }

    constexpr Underlying operator&(const KeyModifierFlags flags) const noexcept
    {
        return value & flags.value;
    }

    constexpr Underlying operator&(const KeyWithModifiers keyWithMods) const noexcept
    {
        return value & keyWithMods.value;
    }

private:
    constexpr explicit KeyWithModifiers(Underlying v)
        : value(v)
    {
    }
    Underlying value {};
};

inline constexpr KeyWithModifiers operator|(Device::KeyCode code, Device::KeyModifier mod)
{
    return KeyWithModifiers(code, mod);
}

inline constexpr KeyWithModifiers operator|(Device::KeyCode code, KeyModifierFlags mods)
{
    return KeyWithModifiers(code, mods);
}

inline constexpr bool operator==(KeyWithModifiers::Underlying u, const KeyWithModifiers keyWithMods)
{
    return keyWithMods == u;
}
