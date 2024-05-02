#include "GameMenuContext.hpp"

#include "machgui/gui.hpp"
#include "system/pathname.hpp"

namespace MachGui
{

GameMenuContext::GameMenuContext(std::string name, MachGuiStartupScreens* pStartupScreens, LoadAnimations load)
    : MachGuiStartupScreenContext(pStartupScreens)
    , name_(name)
{
    if (load == LoadAnimations::Yes)
        animations_.init(pStartupScreens, SysPathName("gui/menu/" + name + "_anims.anm"), MachGui::menuScaleFactor());

    changeBackdrop("gui/menu/" + name + ".bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);
}

} // namespace MachGui
