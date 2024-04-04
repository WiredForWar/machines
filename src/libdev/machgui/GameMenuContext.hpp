#pragma once

#include "machgui/startctx.hpp"

#include "machgui/animatio.hpp"

namespace MachGui
{

class GameMenuContext : public MachGuiStartupScreenContext
{
public:
    explicit GameMenuContext(std::string name, MachGuiStartupScreens* pStartupScreens);

protected:
    MachGuiAnimations animations_;
};

} // namespace MachGui
