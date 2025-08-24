#pragma once

#include "machgui/BindId.hpp"

#include "gui/KeyToString.hpp"

#include "device/KeyBind.hpp"

namespace MachGui
{

class IInputRegistry
{
public:
    using DisplayFormat = Gui::KeysDisplayFormat;

    virtual const KeyBinds& getBinds(BindId id) const = 0;
    virtual const std::string& getBindDisplayString(BindId id, DisplayFormat format = {}) const = 0;
};

IInputRegistry* inputRegistry();

} // namespace MachGui
