/*
 * B U F S C B U T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/bufscbut.hpp"
#include "machgui/prodbank.hpp"
#include "machgui/prodicns.hpp"
#include "machgui/ingame.hpp"
#include "machgui/controlp.hpp"
#include "machgui/internal/mgsndman.hpp"

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachGuiBufferScrollButton::MachGuiBufferScrollButton(
    GuiDisplayable* pParent,
    const Gui::Coord& rel,
    GuiSimpleScrollableList* pList,
    ScrollDir scrollDir,
    MachInGameScreen* pInGameScreen)
    : GuiListObserver(pList)
    , GuiIcon(pParent, rel, getBitmap(scrollDir))
    , scrollDir_(scrollDir)
    , pInGameScreen_(pInGameScreen)
{
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

// virtual
MachGuiBufferScrollButton::~MachGuiBufferScrollButton()
{
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachGuiBufferScrollButton::doBeDepressed(const GuiMouseEvent&)
{
    MachGuiSoundManager::instance().playSound("gui/sounds/igclick.wav");
}

// virtual
void MachGuiBufferScrollButton::doBeReleased(const GuiMouseEvent&)
{
    switch (scrollDir_)
    {
        case FOWARD:
            if (list().canScrollFoward())
                list().scrollFoward();
            break;
        case BACKWARD:
            if (list().canScrollBackward())
                list().scrollBackward();
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachGuiBufferScrollButton::listUpdated()
{
    bool canScroll = list().canScrollBackward() or list().canScrollFoward();

    if (canScroll != isVisible())
    {
        isVisible(canScroll);

        // Clean up control panel if this is being removed.
        if (not canScroll)
        {
            pInGameScreen_->controlPanel().redrawArea(*this);
        }
    }

    changed();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
const GuiBitmap& MachGuiBufferScrollButton::getBitmap() const
{
    return getBitmap(scrollDir_, list().canScrollBackward(), list().canScrollFoward());
}

const GuiBitmap&
MachGuiBufferScrollButton::getBitmap(ScrollDir scrollDir, bool canScrollBackward, bool canScrollForward)
{
    static GuiBitmap scrollLeftBmp(Gui::bitmap(SysPathName("gui/misc/scrolll.bmp")));
    static GuiBitmap scrollLeftHighlightBmp(Gui::bitmap(SysPathName("gui/misc/scrolllh.bmp")));
    static GuiBitmap scrollRightBmp(Gui::bitmap(SysPathName("gui/misc/scrollr.bmp")));
    static GuiBitmap scrollRightHighlightBmp(Gui::bitmap(SysPathName("gui/misc/scrollrh.bmp")));

    if (scrollDir == LEFT)
    {
        if (canScrollBackward)
        {
            return scrollLeftHighlightBmp;
        }
        else
        {
            return scrollLeftBmp;
        }
    }
    else
    {
        if (canScrollForward)
        {
            return scrollRightHighlightBmp;
        }
        else
        {
            return scrollRightBmp;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachGuiBufferScrollButton::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ostream& operator<<(ostream& o, const MachGuiBufferScrollButton& t)
{

    o << "MachGuiBufferScrollButton " << (void*)&t << " start" << std::endl;
    o << "MachGuiBufferScrollButton " << (void*)&t << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* End BUFSCBUT.CPP *************************************************/
