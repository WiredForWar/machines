/*
 * M S G B O X . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/msgbox.hpp"
#include "machgui/startup.hpp"
#include "machgui/menutext.hpp"
#include "machgui/menubutn.hpp"
#include "machgui/gui.hpp"
#include "machgui/internal/strings.hpp"
#include "gui/manager.hpp"
#include "gui/painter.hpp"
#include "machgui/menus_helper.hpp"

MachGuiMessageBoxResponder::MachGuiMessageBoxResponder()
{
}

// virtual
MachGuiMessageBoxResponder::~MachGuiMessageBoxResponder()
{
}

// virtual
bool MachGuiMessageBoxResponder::okPressed()
{
    return true;
}

// virtual
bool MachGuiMessageBoxResponder::cancelPressed()
{
    return true;
}

MachGuiMessageBox::MachGuiMessageBox(MachGuiStartupScreens* pStartupScreens, uint stringResId, MBType mbType)
    : GuiDisplayable(pStartupScreens, Gui::Box(0, 0, 640, 480), GuiDisplayable::LAYER5)
    , animations_(this, animationFile(mbType))
    , mbType_(mbType)
    , pStartupScreens_(pStartupScreens)
{
    // Disable focus on all non-message box controls
    pStartupScreens->messageBoxHasFocus(true);

    new MachGuiMenuText(
        this,
        Gui::Box(203, 157, 484, 293),
        stringResId,
        "gui/menu/largefnt.bmp",
        MachGuiMenuText::LEFT_JUSTIFY);

    displayButtons(pStartupScreens);

    changed();

    useFastSecondDisplay(false);

    TEST_INVARIANT;
}

MachGuiMessageBox::MachGuiMessageBox(
    MachGuiStartupScreens* pStartupScreens,
    uint stringResId,
    MBType mbType,
    const GuiStrings& strs)
    : GuiDisplayable(pStartupScreens, Gui::Box(0, 0, 640, 480), GuiDisplayable::LAYER5)
    , animations_(this, animationFile(mbType))
    , mbType_(mbType)
    , pStartupScreens_(pStartupScreens)
{
    // Disable focus on all non-message box controls
    pStartupScreens->messageBoxHasFocus(true);

    GuiResourceString str(stringResId, strs);
    string wholeStr = str.asString();

    new MachGuiMenuText(
        this,
        Gui::Box(203, 157, 484, 293),
        wholeStr,
        "gui/menu/largefnt.bmp",
        MachGuiMenuText::LEFT_JUSTIFY);

    displayButtons(pStartupScreens);

    changed();

    useFastSecondDisplay(false);

    TEST_INVARIANT;
}

MachGuiMessageBox::~MachGuiMessageBox()
{
    TEST_INVARIANT;

    //delete pAnimations_;

    // Re-enable focus on all non-message box controls
    pStartupScreens_->messageBoxHasFocus(false);
}

// virtual
void MachGuiMessageBox::doDisplay()
{
    auto backdrop = pStartupScreens_->getSharedBitmaps()->getNamedBitmap("backdrop");
    using namespace machgui::helper::menus;
    int menuLeft = x_from_screen_left(pStartupScreens_->getSharedBitmaps()->getWidthOfNamedBitmap(backdrop), 2);
    int menuTop = y_from_screen_bottom(pStartupScreens_->getSharedBitmaps()->getHeightOfNamedBitmap(backdrop), 2);

    const GuiBitmap& boxBitmap = image();
    if (boxBitmap.isNull())
        return;

    const auto msgBoxSize = boxBitmap.size();

    GuiPainter::instance().blit(
        boxBitmap,
        Gui::Box(0, 0, msgBoxSize.width, msgBoxSize.height),
        Gui::Coord(menuLeft, menuTop));
}

void MachGuiMessageBox::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMessageBox& t)
{

    o << "MachGuiMessageBox " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachGuiMessageBox " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachGuiMessageBox::update()
{
    animations_.update();
}

void MachGuiMessageBox::displayButtons(MachGuiStartupScreens* pStartupScreens)
{
    if (mbType_ == MBOKCANCEL)
    {
        MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            Gui::Box(348, 324, 491, 355),
            IDS_MENUBTN_CANCEL,
            MachGuiStartupScreens::BE_CANCEL,
            this);
        MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
            pStartupScreens,
            Gui::Box(175, 324, 319, 355),
            IDS_MENUBTN_OK,
            MachGuiStartupScreens::BE_OK,
            this);
        pOkBtn->hasFocus(true);
        pCancelBtn->escapeControl(true);
        pCancelBtn->setMsgBoxButton(true);
        pOkBtn->setMsgBoxButton(true);
    }
    else if (mbType_ == MBOK)
    {
        MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
            pStartupScreens,
            Gui::Box(348, 324, 491, 355),
            IDS_MENUBTN_OK,
            MachGuiStartupScreens::BE_OK,
            this);
        pOkBtn->hasFocus(true);
        pOkBtn->setMsgBoxButton(true);
    }
    else if (mbType_ == MBYESNO)
    {
        MachGuiMenuButton* pCancelBtn = new MachGuiMenuButton(
            pStartupScreens,
            Gui::Box(348, 324, 491, 355),
            IDS_MENUBTN_NO,
            MachGuiStartupScreens::BE_CANCEL,
            this);
        MachGuiMenuButton* pOkBtn = new MachGuiMenuButton(
            pStartupScreens,
            Gui::Box(175, 324, 319, 355),
            IDS_MENUBTN_YES,
            MachGuiStartupScreens::BE_OK,
            this);
        pOkBtn->hasFocus(true);
        pCancelBtn->escapeControl(true);
        pCancelBtn->setMsgBoxButton(true);
        pOkBtn->setMsgBoxButton(true);
    }
}

const GuiBitmap& MachGuiMessageBox::image() const
{
    if (mbType_ == MBOKCANCEL || mbType_ == MBYESNO)
        return MachGui::okCancelMsgBoxBmp();

    return MachGui::okMsgBoxBmp();
}

// static
SysPathName MachGuiMessageBox::animationFile(MBType mbType)
{
    if (mbType == MBOKCANCEL || mbType == MBYESNO)
        return SysPathName("gui/menu/mb_anim2.anm");

    return SysPathName("gui/menu/mb_anims.anm");
}
/* End MSGBOX.CPP ***************************************************/
