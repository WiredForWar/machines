/*
 * M U S I C V O L . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/musicvol.hpp"

#include "base/diag.hpp"

#include "device/cd.hpp"

MachGuiMusicVolumeSlideBar::MachGuiMusicVolumeSlideBar(
    MachGuiStartupScreens* pStartupScreens,
    GuiDisplayable* pParent,
    Gui::Coord topLeft,
    size_t width)
    : MachGuiSlideBar(pStartupScreens, pParent, topLeft, width, 0, 100)
{
    if (DevCD::instance().supportsVolumeControl())
    {
        size_t theVol = DevCD::instance().volume();
        RICHARD_STREAM("Volume initialized to " << theVol << std::endl);
        value(theVol);
    }

    TEST_INVARIANT;
}

MachGuiMusicVolumeSlideBar::~MachGuiMusicVolumeSlideBar()
{
    TEST_INVARIANT;
}

void MachGuiMusicVolumeSlideBar::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

// virtual
void MachGuiMusicVolumeSlideBar::valueChanged(float value)
{
    RICHARD_STREAM("CD Vol changing with value " << value << std::endl);
    if (DevCD::instance().supportsVolumeControl())
        DevCD::instance().volume(value + 0.5 /*stop rounding errors from slowly reducing volume*/);
}

std::ostream& operator<<(std::ostream& o, const MachGuiMusicVolumeSlideBar& t)
{

    o << "MachGuiMusicVolumeSlideBar " << (void*)&t << " start" << std::endl;
    o << "MachGuiMusicVolumeSlideBar " << (void*)&t << " end" << std::endl;

    return o;
}

/* End MUSICVOL.CPP *************************************************/
