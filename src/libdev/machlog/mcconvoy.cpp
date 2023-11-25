/*
 * M C C O N V O Y . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machlog/mcconvoy.hpp"
#include "mathex/poly2d.hpp"
#include "phys/motchunk.hpp"
#include "machlog/internal/mcconvoi.hpp"
#include "machlog/mcmotseq.hpp"
#include "machlog/machine.hpp"
#include "machphys/machine.hpp"

#ifndef _INLINE
#include "machlog/mcconvoy.ipp"
#endif

MachLogMachineConvoy::MachLogMachineConvoy(MachLogMachineMotionSequencer* pSequencer)
    : pImpl_(new MachLogMachineConvoyImpl())
{
    CB_MachLogMachineConvoy_DEPIMPL();

    PRE(pSequencer != nullptr);

    minTopSpeedUpToDate_ = false;

    // Set up for a reasonable amount of memory
    sequencers_.reserve(8);

    // Add the machine
    sequencers_.push_back(pSequencer);

    TEST_INVARIANT;
}

MachLogMachineConvoy::~MachLogMachineConvoy()
{
    TEST_INVARIANT;

    delete pImpl_;
}

void MachLogMachineConvoy::CLASS_INVARIANT
{
}

std::ostream& operator<<(std::ostream& o, const MachLogMachineConvoy& t)
{

    o << "MachLogMachineConvoy " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogMachineConvoy " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

void MachLogMachineConvoy::add(MachLogMachineMotionSequencer* pSequencer)
{
    CB_MachLogMachineConvoy_DEPIMPL();

    PRE(! pSequencer->isInConvoy());

    // Add to the vector
    sequencers_.push_back(pSequencer);

    // Top speed out of date
    minTopSpeedUpToDate_ = false;
}

void MachLogMachineConvoy::remove(MachLogMachineMotionSequencer* pSequencer)
{
    CB_MachLogMachineConvoy_DEPIMPL();

    PRE(pSequencer->isInConvoy() && &(pSequencer->convoy()) == this);

    // Find the entry in the list
    Sequencers::iterator it = find(sequencers_.begin(), sequencers_.end(), pSequencer);
    ASSERT(it != sequencers_.end(), "");

    // Erase it
    sequencers_.erase(it);

    // Top speed out of date
    minTopSpeedUpToDate_ = false;
}

MATHEX_SCALAR MachLogMachineConvoy::minTopSpeed() const
{
    CB_DEPIMPL(const MachLogMachineConvoy::Sequencers, sequencers_);
    CB_DEPIMPL(MATHEX_SCALAR, minTopSpeed_);
    CB_DEPIMPL(bool, minTopSpeedUpToDate_);

    PRE(sequencers_.size() != 0);

    // Update the minimum top speed
    if (! minTopSpeedUpToDate_)
    {
        // Initialise to first speed
        Sequencers::const_iterator cit = sequencers_.begin();
        minTopSpeed_ = (*cit)->physMachine().maxTranslationSpeed();

        while (++cit != sequencers_.end())
        {
            MATHEX_SCALAR speed = (*cit)->physMachine().maxTranslationSpeed();
            if (speed < minTopSpeed_)
                minTopSpeed_ = speed;
        }

        minTopSpeedUpToDate_ = true;
    }

    return minTopSpeed_;
}

const MachLogMachineConvoy::Sequencers& MachLogMachineConvoy::sequencers() const
{
    CB_MachLogMachineConvoy_DEPIMPL();

    return sequencers_;
}

/* End MCCONVOY.CPP *************************************************/
