#include "GameMenuContext.hpp"

#include "machgui/gui.hpp"
#include "system/PathName.hpp"
#include "system/VFS.hpp"

namespace MachGui
{

GameMenuContext::GameMenuContext(std::string name, MachGuiStartupScreens* pStartupScreens, LoadAnimations load)
    : MachGuiStartupScreenContext(pStartupScreens)
    , name_(name)
{
    if (load == LoadAnimations::Yes)
        animations_.init(pStartupScreens, SysPathName("gui/menu/" + name + "_anims.anm"), MachGui::menuScaleFactor());

    changeBackdrop("gui/menu/" + name + ".bmp");
    const std::string widePath = "gui/menu/wide/" + name + ".png";
    if (SysPathName::existsAsFile(System::findFile(widePath)))
        pStartupScreens->getSharedBitmaps()->createUpdateNamedBitmap(
            "wide-backdrop", "gui/menu/wide/" + name + ".bmp", 1);

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);
}

} // namespace MachGui
