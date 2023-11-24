/*
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machgui/bufscbut.hpp"
#include "machgui/hwrebank.hpp"
#include "machgui/hwrbicns.hpp"
#include "machgui/gui.hpp"
#include "machgui/ingame.hpp"
#include "machgui/controlp.hpp"
#include "machlog/machlog.hpp"
#include "machlog/hwlab.hpp"
#include "machlog/resitem.hpp"
#include "gui/gui.hpp"
#include "gui/progress.hpp"
#include "gui/icon.hpp"
#include "device/butevent.hpp"
#include "machgui/internal/mgsndman.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////
// ******************************************* Code for MachHWResearchBank *************************************
//////

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachHWResearchBank::MachHWResearchBank(
    GuiDisplayable* pParent,
    const Gui::Boundary& relativeBoundary,
    MachLogHardwareLab* pHardwareLab,
    MachInGameScreen* pInGameScreen)
    : GuiDisplayable(pParent, relativeBoundary, GuiDisplayable::LAYER3)
    , pHardwareLab_(pHardwareLab)
    , pIcons_(nullptr)
    , observingLab_(false)
{

    // Construct the icon sequence depicting the queue
    Gui::Box iconsArea(
        MachGuiBufferScrollButton::width(),
        0,
        MachHWResearchBankIcons::reqWidth() + MachGuiBufferScrollButton::width(),
        MachHWResearchBankIcons::reqHeight());

    pIcons_ = new MachHWResearchBankIcons(this, iconsArea, pHardwareLab_, pInGameScreen);

    // Become an observer of the lab
    pHardwareLab_->attach(this);
    observingLab_ = true;

    // Create and display a build progress indicator if required
    updateProgress();

    pScrollLeft_ = new MachGuiBufferScrollButton(
        this,
        Gui::Coord(0, 0),
        pIcons_,
        MachGuiBufferScrollButton::LEFT,
        pInGameScreen);
    pScrollRight_ = new MachGuiBufferScrollButton(
        this,
        Gui::Coord(MachGuiBufferScrollButton::width() + MachHWResearchBankIcons::reqWidth(), 0),
        pIcons_,
        MachGuiBufferScrollButton::RIGHT,
        pInGameScreen);

    updateQueueIcons();

    TEST_INVARIANT;
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachHWResearchBank::~MachHWResearchBank()
{
    TEST_INVARIANT;

    // Cease observing the factory
    if (observingLab_)
        pHardwareLab_->detach(this);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBank::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBank::updateQueueIcons()
{
    pIcons_->updateIcons();
    updateProgress();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachHWResearchBank::updateProgress()
{
    // If the hw lab is researching, we'll need a progress bar
    MachLogResearchItem* pCurrentItem;
    if (pHardwareLab_->currentlyResearching(&pCurrentItem))
    {
        // Sets its current level
        double percentComplete = 100.0
            * (double(pCurrentItem->amountResearched(pHardwareLab_->race())) / double(pCurrentItem->researchCost()));

        pIcons_->updateProgress(percentComplete);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchBank::doDisplay()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MachLogHardwareLab& MachHWResearchBank::hardwareLab()
{
    PRE(pHardwareLab_ != nullptr);

    return *pHardwareLab_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachHWResearchBank::domainDeleted(W4dDomain*)
{
    // Do nothing
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
bool MachHWResearchBank::beNotified(W4dSubject*, W4dSubject::NotificationEvent event, int clientData)
{
    // Since we only ever observe the factory, it must be the subject.
    switch (event)
    {
        case W4dSubject::DELETED:
            {
                observingLab_ = false;
                break;
            }

        case W4dSubject::CLIENT_SPECIFIC:
            {
                switch (clientData)
                {
                    case MachLog::HW_RESEARCH_COMPLETE:
                        {
                            // Update the research queue icon display
                            updateQueueIcons();
                            break;
                        }
                }
                break;
            }
    }

    return observingLab_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchBank::reqWidth()
{
    return (
        MachGuiBufferScrollButton::width() + MachHWResearchBankIcons::reqWidth() + MachGuiBufferScrollButton::width());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachHWResearchBank::reqHeight()
{
    return (MachHWResearchBankIcons::reqHeight());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachHWResearchBank& t)
{

    o << "MachHWResearchBank " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachHWResearchBank " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* End *************************************************/
