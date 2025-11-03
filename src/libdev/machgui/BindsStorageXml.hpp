#pragma once

#include "IBindsStorage.hpp"

namespace MachGui
{

class BindsStorageXml : public IBindsStorage
{
public:
    void read(BindId id, KeyBinds* binds) override;
    void write(BindId id, const KeyBinds& binds) override;
    void sync() override;
};

} // namespace MachGui
