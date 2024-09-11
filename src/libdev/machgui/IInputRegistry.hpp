#pragma once

#include <string>

#include "device/butevent.hpp"
#include "gui/KeyWithModifiers.hpp"
#include "utility/Array.hpp"

namespace MachGui
{

class Shortcut
{
public:

    Shortcut() = default;
    Shortcut(std::initializer_list<KeyWithModifiers> codes)
        : keys_(codes)
    {
    }

    bool matches(const KeyWithModifiers& key) const { return keys_.contains(key); }
    bool matches(const DevButtonEvent& be) const;

protected:
    Utility::Array<KeyWithModifiers, 3> keys_;
};

class IInputRegistry
{
public:
    using ActionId = std::string;

    virtual const Shortcut& getShortcut(ActionId id) = 0;
};

IInputRegistry* inputRegistry();

} // namespace MachGui
