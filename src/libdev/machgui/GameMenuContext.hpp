#pragma once

#include "machgui/StartupScreenContext.hpp"

#include "machgui/Animation.hpp"

namespace MachGui
{

class GameMenuContext : public MachGuiStartupScreenContext
{
protected:
    enum class LoadAnimations
    {
        Yes,
        No
    };

public:
    explicit GameMenuContext(
        std::string name,
        MachGuiStartupScreens* pStartupScreens,
        LoadAnimations load = LoadAnimations::Yes);

protected:
    MachGuiAnimations animations_;
    std::string name_;
};

} // namespace MachGui
