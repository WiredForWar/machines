#pragma once

#include "system/registry.hpp"

#include <string>
#include <vector>

class MachGuiOptionsManager : public ISysRegistryObserver
{
public:
    MachGuiOptionsManager();
    ~MachGuiOptionsManager() override;

    void initializeCursor();
    void initializeLanguage();

    std::vector<std::string> availableLanguages() const;

    void onChanges() override;

private:
    void recreatePhysMarkers();
};
