/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machgui/HWResearchBankIcons.hpp"
#include "machgui/NewResearchIcon.hpp"
#include "machgui/gui.hpp"
#include "machgui/InGameScreen.hpp"
#include "machgui/ControlPanel.hpp"
#include "machlog/Actors/HardwareLab.hpp"
#include "gui/Manager.hpp"
#include "ctl/PtrVector.hpp"

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachHWResearchBankIcons::MachHWResearchBankIcons(
    GuiDisplayable* pParent,
    const Gui::Box& area,
    MachLogHardwareLab* pHardwareLab,
    MachInGameScreen* pInGameScreen)
    : GuiSimpleScrollableList(pParent, area, MachHWResearchBankIcon::reqWidth(), MachHWResearchBankIcon::reqHeight(), 1)
    , pHardwareLab_(pHardwareLab)
    , pInGameScreen_(pInGameScreen)
{
    // Add the icons
    updateIcons();

    TEST_INVARIANT;
}

void MachHWResearchBankIcons::onIconClicked(MachHWResearchBankIcon* pIcon)
{
    pHardwareLab_->removeResearchItem(*pIcon->researchItem());

    updateIcons();
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachHWResearchBankIcons::~MachHWResearchBankIcons()
{
    TEST_INVARIANT;
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBankIcons::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachHWResearchBankIcons& t)
{

    o << "MachHWResearchBankIcons " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachHWResearchBankIcons " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBankIcons::updateIcons()
{
    // Delete the existing icons
    deleteAllChildren();

    // Get the current research queue
    const MachLogResearchTree::ResearchItems& queue = pHardwareLab_->currentResearchQueue();

    MachPhys::Race race = pHardwareLab_->race();

    // Iterate through the queue and add an icon for each one
    for (const MachLogResearchItem* pResearchItem : queue)
    {
        MachHWResearchBankIcon* pIcon = new MachHWResearchBankIcon(this, pInGameScreen_, pResearchItem, race);
        pIcon->setMouseClickHandler([this, pIcon] { onIconClicked(pIcon); });
    }

    // Ensure redisplayed
    childrenUpdated();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBankIcons::doDisplay()
{
    pInGameScreen_->controlPanel().redrawAreaImmediate(*this);
    GuiSimpleScrollableList::doDisplay();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchBankIcons::reqHeight()
{
    return MachHWResearchBankIcon::reqHeight();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchBankIcons::reqWidth()
{
    return (3 * MachHWResearchBankIcon::reqWidth());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBankIcons::updateProgress(float percentageComplete)
{
    if (! children().empty() && ! canScrollBackward())
    {
        MachHWResearchBankIcon* pHeadOfList = _REINTERPRET_CAST(MachHWResearchBankIcon*, children().front());
        pHeadOfList->updateProgress(percentageComplete);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* End *************************************************/
