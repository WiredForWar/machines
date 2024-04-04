/*
 * S T A R T C T X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachGuiStartupScreenContext

    A brief description of the class should go in here
*/

#ifndef _MACHGUI_STARTCTX_HPP
#define _MACHGUI_STARTCTX_HPP

#include "base/base.hpp"
#include "machgui/startup.hpp"

class GuiKeyEvent;

class MachGuiStartupScreenContext
// Canonical form revoked
{
public:
    explicit MachGuiStartupScreenContext(MachGuiStartupScreens* pStartupScreens);
    virtual ~MachGuiStartupScreenContext();

    // Called every frame.
    virtual void update();

    // When context is about to change this function is called to make sure that
    // switching context is acceptable ( e.g. not enough info has been entered on
    // this screen to proceed so return false ).
    virtual bool okayToSwitchContext();

    // Called when a button is pressed if the default processing by MachGuiStartupScreens
    // does not "use" the button event.
    virtual void buttonEvent(MachGui::ButtonEvent);

    // Called if the startup screens does not use the GuiKeyEvent.
    virtual bool doHandleKeyEvent(const GuiKeyEvent&);

    void CLASS_INVARIANT;

protected:
    MachGuiStartupScreens* pStartupScreens_;

    std::pair<int, int> getBackdropTopLeft();
    void changeBackdrop(const std::string& newBackdrop);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachGuiStartupScreenContext& t);

    MachGuiStartupScreenContext(const MachGuiStartupScreenContext&);
    MachGuiStartupScreenContext& operator=(const MachGuiStartupScreenContext&);
};

#endif

/* End STARTCTX.HPP *************************************************/
