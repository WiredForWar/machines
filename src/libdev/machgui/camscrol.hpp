/*
 * C A M S C R O L . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiCameraScrollArea

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_CAMSCROL_HPP
#define _MACHGUI_CAMSCROL_HPP

#include "base/base.hpp"
#include "gui/displaya.hpp"

class MachCameras;
class MachInGameScreen;

class MachGuiCameraScrollArea : public GuiDisplayable
// Canonical form revoked
{
public:
    enum POSITION
    {
        Top,
        Bottom,
        Right,
        Left
    };

    MachGuiCameraScrollArea(
        GuiDisplayable* pParent,
        const Gui::Box& rel,
        POSITION pos,
        MachCameras*,
        MachInGameScreen*);
    ~MachGuiCameraScrollArea() override;

    void CLASS_INVARIANT;

protected:
    void doHandleContainsMouseEvent(const GuiMouseEvent&) override;
    void doHandleMouseExitEvent(const GuiMouseEvent&) override;

    void doDisplay() override;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiCameraScrollArea& t);

    MachGuiCameraScrollArea(const MachGuiCameraScrollArea&);
    MachGuiCameraScrollArea& operator=(const MachGuiCameraScrollArea&);

    POSITION pos_;
    MachCameras* pCameras_;
    MachInGameScreen* pInGameScreen_;
};

#endif

/* End CAMSCROL.HPP *************************************************/
