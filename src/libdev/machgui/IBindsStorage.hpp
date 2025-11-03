#pragma once

#include "BindId.hpp"

class KeyBinds;

namespace MachGui
{

class IBindsStorage
{
public:
    virtual ~IBindsStorage() = default;
    virtual void read(BindId id, KeyBinds* binds) = 0;
    virtual void write(BindId id, const KeyBinds& binds) = 0;
    virtual void sync() = 0;
};

} // namespace MachGui
