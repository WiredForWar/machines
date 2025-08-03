#pragma once

#include "system/registry.hpp"

class MachGuiOptionsManager : public ISysRegistryObserver
{
public:
    MachGuiOptionsManager();
    ~MachGuiOptionsManager() override;

    void initializeCursor();

    void onChanges() override;
};
