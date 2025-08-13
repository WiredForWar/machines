#pragma once

#include "device/KeyWithModifiers.hpp"

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
