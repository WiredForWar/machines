/*
 * C T X M M E N U . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/ctxmmenu.hpp"
#include "machgui/ui/MenuButton.hpp"
#include "machgui/ui/MenuText.hpp"
#include "machgui/startup.hpp"
#include "machgui/animatio.hpp"
#include "machgui/msgbox.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/font.hpp"
#include "system/pathname.hpp"

///////////////////////////////////////////////////////////////////////////////////////////

class MachGuiMainMenuMessageBoxResponder : public MachGuiMessageBoxResponder
{
public:
    MachGuiMainMenuMessageBoxResponder(MachGuiCtxMainMenu* pMainMenuCtx)
        : pMainMenuCtx_(pMainMenuCtx)
    {
    }

    bool okPressed() override
    {
        pMainMenuCtx_->quit();
        return true;
    }

private:
    MachGuiCtxMainMenu* pMainMenuCtx_;
};

///////////////////////////////////////////////////////////////////////////////////////////

MachGuiCtxMainMenu::MachGuiCtxMainMenu(MachGuiStartupScreens* pStartupScreens)
    : MachGui::GameMenuContext("sa", pStartupScreens)
{
    // Access all menu fonts to make sure they are loaded ready for other context to use. This will
    // stop annoying pauses as other context access and use fonts for the first time.
    GuiBmpFont::getFont("gui/menu/smallfnt.bmp");
    GuiBmpFont::getFont("gui/menu/smaldfnt.bmp");
    GuiBmpFont::getFont("gui/menu/largefnt.bmp");
    GuiBmpFont::getFont("gui/menu/largdfnt.bmp");

#ifdef DEMO
    new MachGuiMenuButton(
        pStartupScreens,
        Gui::Box(368, 40, 527, 75),
        IDS_MENUBTN_CAMPAIGN,
        MachGui::ButtonEvent::CAMPAIGN);
    new MachGuiMenuButton(
        pStartupScreens,
        Gui::Box(368, 130, 527, 165),
        IDS_MENUBTN_MULTIPLAYER,
        MachGui::ButtonEvent::MULTIPLAYER);
    new MachGuiMenuButton(
        pStartupScreens,
        Gui::Box(368, 217, 527, 252),
        IDS_MENUBTN_SKIRMISH,
        MachGui::ButtonEvent::SKIRMISH);
    new MachGuiMenuButton(
        pStartupScreens,
        Gui::Box(368, 297, 527, 332),
        IDS_MENUBTN_OPTIONS,
        MachGui::ButtonEvent::OPTIONS);
    MachGuiMenuButton* pExitBtn = new MachGuiMenuButton(
        pStartupScreens,
        Gui::Box(117, 410, 275, 445),
        IDS_MENUBTN_EXIT,
        MachGui::ButtonEvent::DUMMY_OK);

#else
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(368, 40, 527, 75),
        IDS_MENUBTN_SINGLEPLAYER,
        MachGui::ButtonEvent::SINGLEPLAYER);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(368, 130, 527, 165),
        IDS_MENUBTN_MULTIPLAYER,
        MachGui::ButtonEvent::MULTIPLAYER);
    new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(368, 217, 527, 252),
        IDS_MENUBTN_OPTIONS,
        MachGui::ButtonEvent::OPTIONS);
    MachGuiMenuButton* pExitBtn = new MachGuiMenuButton(
        pStartupScreens,
        pStartupScreens,
        Gui::Box(368, 297, 527, 332),
        IDS_MENUBTN_EXIT,
        MachGui::ButtonEvent::DUMMY_OK);

#endif

    // Create exit button, responds to escape...
    pExitBtn->escapeControl(true);

    new MachGuiMenuText(pStartupScreens, Gui::Box(97, 32, 243, 122), IDS_MENU_MAINMENU, "gui/menu/largefnt.bmp");

    changeBackdrop("gui/menu/sa.bmp");

    pStartupScreens->cursorOn(true);
    pStartupScreens->desiredCdTrack(MachGuiStartupScreens::MENU_MUSIC);

    TEST_INVARIANT;
}

MachGuiCtxMainMenu::~MachGuiCtxMainMenu()
{
    TEST_INVARIANT;
}

void MachGuiCtxMainMenu::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiCtxMainMenu& t)
{

    o << "MachGuiCtxMainMenu " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiCtxMainMenu " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiCtxMainMenu::update()
{
    if (quit_)
    {
        pStartupScreens_->buttonAction(MachGui::ButtonEvent::EXIT);
    }
    else
    {
        animations_.update();
    }
}

// virtual
bool MachGuiCtxMainMenu::okayToSwitchContext()
{
    if (pStartupScreens_->lastButtonEvent() == MachGui::ButtonEvent::DUMMY_OK)
    {
        pStartupScreens_->displayMsgBox(IDS_MENUMSG_QUIT, new MachGuiMainMenuMessageBoxResponder(this), true);
        return false;
    }

    return true;
}

void MachGuiCtxMainMenu::quit()
{
    quit_ = true;
}

/* End CTXMMENU.CPP *************************************************/
