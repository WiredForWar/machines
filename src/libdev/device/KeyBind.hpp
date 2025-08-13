#pragma once

#include "device/KeyWithModifiers.hpp"
#include "utility/Array.hpp"

class KeyBind
{
public:
    KeyWithModifiers keyWithMods{};
    KeyModifierFlags releasedModifiers{};

    Device::KeyCode keyCode() const;

    bool matches(const KeyWithModifiers& key) const;
    bool modifiersMatch(const KeyWithModifiers& key) const;
    bool keysMatch(const KeyWithModifiers& key) const;
};

class KeyBinds : public Utility::Array<KeyBind, 3>
{
public:
    using Array::Array;

    bool matches(const KeyWithModifiers& key) const;
};

inline Device::KeyCode KeyBind::keyCode() const
{
    return keyWithMods.keyCode();
}

inline bool KeyBind::matches(const KeyWithModifiers& key) const
{
    return modifiersMatch(key) && keysMatch(key);
}

inline bool KeyBind::modifiersMatch(const KeyWithModifiers& key) const
{
    const KeyModifierFlags modifiers = key.modifiers();
    if (modifiers & releasedModifiers)
        return false;

    const auto bindMods = keyWithMods.modifiers();
    return (modifiers & bindMods) == bindMods;
}

inline bool KeyBind::keysMatch(const KeyWithModifiers& key) const
{
    return key.keyCode() == keyWithMods.keyCode();
}

inline bool KeyBinds::matches(const KeyWithModifiers& key) const
{
    for (const KeyBind bind : *this)
    {
        if (bind.matches(key))
            return true;
    }

    return false;
}
