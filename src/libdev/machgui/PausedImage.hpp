/*
 * P A U S E D I M A G E . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiPausedImage

    A GuiDisplayable overlay that draws the "paused", "victory", "defeat",
    or "network busy" bitmap centred within the parent's viewport area.
    Used by both MachWorldViewWindow and MachGuiFirstPerson.
*/

#ifndef _MACHGUI_PAUSEDIMAGE_HPP
#define _MACHGUI_PAUSEDIMAGE_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"

class MachInGameScreen;

class MachGuiPausedImage : public GuiDisplayable
{
public:
    MachGuiPausedImage(GuiDisplayable* pParent, MachInGameScreen* pInGameScreen);

    void doDisplay() override;

private:
    MachGuiPausedImage(const MachGuiPausedImage&);
    MachGuiPausedImage& operator=(const MachGuiPausedImage&);

    MachInGameScreen* pInGameScreen_;
};

#endif

/* End PAUSEDIMAGE.HPP **********************************************/
