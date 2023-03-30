/*
 * P R O D B A N K . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "stdlib/string.hpp"
#include "machgui/prodbank.hpp"
#include "machgui/bufscbut.hpp"
#include "machgui/prodicns.hpp"
#include "machgui/gui.hpp"
#include "machgui/ingame.hpp"
#include "machlog/machlog.hpp"
#include "machlog/machine.hpp"
#include "machlog/factory.hpp"
#include "gui/gui.hpp"
#include "device/butevent.hpp"

/* ////////////////////////////////////////////// constructor /////////////////////////////////////////////////// */

MachProductionBank::MachProductionBank(
    GuiDisplayable* pParent,
    const Gui::Boundary& relativeBoundary,
    MachLogFactory* pFactory,
    MachInGameScreen* pInGameScreen)
    : GuiDisplayable(pParent, relativeBoundary, GuiDisplayable::LAYER3)
    , pFactory_(pFactory)
    , pIcons_(nullptr)
    , pProgressBar_(nullptr)
    , observingFactory_(false)
{

    // Construct the icon sequence depicting the queue
    Gui::Box iconsArea(
        MachGuiBufferScrollButton::width(),
        0,
        MachProductionIcons::width() + MachGuiBufferScrollButton::width(),
        MachProductionIcons::height());

    pIcons_ = _NEW(MachProductionIcons(this, iconsArea, pFactory, pInGameScreen));

    // Become an observer of the factory
    pFactory_->attach(this);
    observingFactory_ = true;

    // Create and display a build progress indicator if required
    updateProgress();

    pScrollLeft_ = _NEW(MachGuiBufferScrollButton(
        this,
        Gui::Coord(0, 0),
        pIcons_,
        MachGuiBufferScrollButton::LEFT,
        pInGameScreen));
    pScrollRight_ = _NEW(MachGuiBufferScrollButton(
        this,
        Gui::Coord(MachGuiBufferScrollButton::width() + MachProductionIcons::width(), 0),
        pIcons_,
        MachGuiBufferScrollButton::RIGHT,
        pInGameScreen));

    updateQueueIcons();

    TEST_INVARIANT;
}

/* /////////////////////////////////////////////// destructor /////////////////////////////////////////////////// */

MachProductionBank::~MachProductionBank()
{
    TEST_INVARIANT;

    // Cease observing the factory
    if (observingFactory_)
        pFactory_->detach(this);
}

/* ////////////////////////////////////////////////////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachProductionBank::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
int MachProductionBank::requiredHeight()
{
    return 42 + 12; // Todo : remove hardcoded values
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachProductionBank::updateQueueIcons()
{
    pIcons_->updateIcons();
    updateProgress();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachProductionBank::updateProgress()
{
    // If the factory is building, we'll need a progress bar
    MachLog::ObjectType objectType;
    int subType;
    MachLogMachine::Level hwLevel;
    MachLogMachine::Level swLevel;
    MachPhys::WeaponCombo wc;

    // NB the arguments are for return value, passed by non-const reference.
    if (pFactory_->currentlyBuilding())
    {
        pIcons_->updateProgress(pFactory_->percentComplete());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachProductionBank::doDisplay()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MachLogFactory& MachProductionBank::factory()
{
    return *pFactory_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachProductionBank::domainDeleted(W4dDomain*)
{
    // Do nothing
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachProductionBank::width()
{
    return (MachGuiBufferScrollButton::width() + MachProductionIcons::width() + MachGuiBufferScrollButton::width());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// static
size_t MachProductionBank::height()
{
    return (MachProductionIcons::height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
bool MachProductionBank::beNotified(W4dSubject*, W4dSubject::NotificationEvent event, int clientData)
{
    // Since we only ever observe the factory, it must be the subject.
    switch (event)
    {
        case W4dSubject::DELETED:
            {
                observingFactory_ = false;
                break;
            }

        case W4dSubject::CLIENT_SPECIFIC:
            {
                switch (clientData)
                {
                    case MachLog::MACHINE_BUILT:
                        {
                            // Update the production queue icon display
                            updateQueueIcons();
                            break;
                        }
                }
                break;
            }
    }

    return observingFactory_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachProductionBank& t)
{

    o << "MachProductionBank " << (void*)&t << " start" << std::endl;
    o << "MachProductionBank " << (void*)&t << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////
// ************************************** Header and Code for MachGuiBufferScrollButton ********************************
//////

// ************************************************************************************************************** //

/* End PRODBANK.CPP *************************************************/
