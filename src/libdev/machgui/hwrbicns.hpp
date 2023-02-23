/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachHWResearchBankIcons

    The research queue for a hardware lab depicted as a sequence of icons
*/

#ifndef _MACHGUI_HWRBICNS_HPP
#define _MACHGUI_HWRBICNS_HPP

#include "base/base.hpp"
#include "gui/scrolist.hpp"
#include "gui/gui.hpp"

// Forward refs
class GuiButton;
class MachInGameScreen;
class MachLogHardwareLab;

// orthodox canonical (revoked)
class MachHWResearchBankIcons : public GuiSimpleScrollableList
{
public:
    // ctor. Located in pParent, covering area.
    // In game screen passed.
    MachHWResearchBankIcons(
        GuiDisplayable* pParent,
        const Gui::Box& area,
        MachLogHardwareLab* pHardwareLab,
        MachInGameScreen* pInGameScreen);

    // dtor.
    ~MachHWResearchBankIcons() override;

    // Set up the list of icons
    void updateIcons();

    static size_t reqHeight();
    static size_t reqWidth();

    void updateProgress(float percentageComplete);

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachHWResearchBankIcons& t);

protected:
    void doDisplay() override;

private:
    // Operations deliberately revoked
    MachHWResearchBankIcons(const MachHWResearchBankIcons&);
    MachHWResearchBankIcons& operator=(const MachHWResearchBankIcons&);
    bool operator==(const MachHWResearchBankIcons&);

    void onIconClicked(GuiButton* pIcon);

    // data members
    MachLogHardwareLab* pHardwareLab_; // The hw lab whose research queue is depicted
    MachInGameScreen* pInGameScreen_; // The gui root
};

#endif

/* End *************************************************/
