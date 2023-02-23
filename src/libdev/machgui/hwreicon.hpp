/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachHWResearchIcon

    The icon that appears in the hardware research menu.
    Clicking the icon adds an item to the lab's research queue.
*/

#ifndef _MACHGUI_HWREICON_HPP
#define _MACHGUI_HWREICON_HPP

#include "base/base.hpp"
#include "gui/icon.hpp"
#include "machlog/machlog.hpp"

// Forward refs
class GuiDisplayable;
class MachInGameScreen;
class MachLogHardwareLab;
class MachLogResearchItem;
class MachHWResearchBank;

// orthodox canonical (revoked)
class MachHWResearchIcon : public GuiIcon
{
public:
    // Owning GuiDisplayable is pParent.
    // Has access to pInGameScreen.
    // pHWResearchBank represents the state of the research queue.
    // The hardware lab whose research queue is being edited is pHardwareLab.
    MachHWResearchIcon(
        GuiDisplayable* pParent,
        MachInGameScreen* pInGameScreen,
        MachHWResearchBank* pHWResearchBank,
        MachLogHardwareLab* pHardwareLab,
        MachLogResearchItem* pResearchItem);

    // dtor.
    ~MachHWResearchIcon() override;

    // inherited from GuiButton...
    void doBeDepressed(const GuiMouseEvent& rel) override;
    void doBeReleased(const GuiMouseEvent& rel) override;

    void CLASS_INVARIANT;

    static size_t reqWidth();
    static size_t reqHeight();

protected:
    void doHandleMouseEnterEvent(const GuiMouseEvent& mouseEvent) override;
    void doHandleMouseExitEvent(const GuiMouseEvent& mouseEvent) override;

private:
    // Operations deliberately revoked
    MachHWResearchIcon(const MachHWResearchIcon&);
    MachHWResearchIcon& operator=(const MachHWResearchIcon&);
    bool operator==(const MachHWResearchIcon&);

    friend ostream& operator<<(ostream& o, const MachHWResearchIcon& t);

    // Data members
    MachInGameScreen* pInGameScreen_; // The ingame screen
    MachHWResearchBank* pHWResearchBank_; // Represents the production queue
    MachLogHardwareLab* pHardwareLab_; // The factory being edited
    MachLogResearchItem* pResearchItem_; // The research item represented
};

#endif

/* End  *************************************************/
