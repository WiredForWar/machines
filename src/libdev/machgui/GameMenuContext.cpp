#include "GameMenuContext.hpp"

#include "system/pathname.hpp"

namespace MachGui
{

GameMenuContext::GameMenuContext(std::string name, MachGuiStartupScreens* pStartupScreens)
    : MachGuiStartupScreenContext(pStartupScreens)
    , animations_(pStartupScreens, SysPathName("gui/menu/" + name + "_anims.anm"))
{
}

} // namespace MachGui
