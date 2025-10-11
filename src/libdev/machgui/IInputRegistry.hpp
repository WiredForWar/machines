#pragma once

#include "machgui/BindId.hpp"

#include "device/KeyBind.hpp"

namespace MachGui
{

class IInputRegistry
{
public:
    virtual const KeyBinds& getBinds(BindId id) const = 0;
    virtual const std::string& getBindDisplayString(BindId id) const = 0;
};

IInputRegistry* inputRegistry();

} // namespace MachGui
