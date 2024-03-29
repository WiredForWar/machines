/*
 * M C D M G D A T . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/mcdmgdat.hpp"
#include "machphys/machine.hpp"
#include "machphys/machburn.hpp"
#include "machphys/machdmg.hpp"

MachPhysMachineDamageData::MachPhysMachineDamageData(MachPhysMachine* pTarget)
    : pTargetMachine_(pTarget)
    , pMachineBurning_(nullptr)
    , pMachineDamage_(nullptr)
{
    TEST_INVARIANT;
}

MachPhysMachineDamageData::~MachPhysMachineDamageData()
{
    delete pMachineBurning_;
    delete pMachineDamage_;

    TEST_INVARIANT;
}

void MachPhysMachineDamageData::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysMachineDamageData& t)
{

    o << "MachPhysMachineDamageData " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachPhysMachineDamageData " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

PER_DEFINE_PERSISTENT(MachPhysMachineDamageData);
MachPhysMachineDamageData::MachPhysMachineDamageData(PerConstructor)
{
}

void perWrite(PerOstream& ostr, const MachPhysMachineDamageData& t)
{
    ostr << t.pTargetMachine_;
    ostr << t.pMachineBurning_;
    ostr << t.pMachineDamage_;
}

void perRead(PerIstream& istr, MachPhysMachineDamageData& t)
{
    istr >> t.pTargetMachine_;
    istr >> t.pMachineBurning_;
    istr >> t.pMachineDamage_;
}

bool MachPhysMachineDamageData::isDamaged() const
{
    return pMachineDamage_ != nullptr;
}

void MachPhysMachineDamageData::damageLevel(const double& percent)
{
    if (percent > 0)
    {
        if (! isDamaged())
            pMachineDamage_ = new MachPhysMachineDamage(pTargetMachine_);

        pMachineDamage_->damageLevel(percent);
    }
}

const double& MachPhysMachineDamageData::damageLevel() const
{
    static const double zero = 0;
    return (pMachineDamage_ == nullptr ? zero : pMachineDamage_->damageLevel());
}

void MachPhysMachineDamageData::updateDamageLevel()
{
    if (isDamaged())
    {
        pMachineDamage_->update();
        if (damageLevel() >= 100)
            pTargetMachine_->beDestroyed();
    }
}

MachPhysMachineBurning& MachPhysMachineDamageData::machineBurning()
{
    if (! isBurning())
        pMachineBurning_ = new MachPhysMachineBurning(pTargetMachine_);

    return *pMachineBurning_;
}

void MachPhysMachineDamageData::finishBurning()
{
    // Check for finished burning
    if (isBurningFinished())
    {
        delete pMachineBurning_;
        pMachineBurning_ = nullptr;
    }
}

bool MachPhysMachineDamageData::isBurning() const
{
    return pMachineBurning_ != nullptr;
}

bool MachPhysMachineDamageData::isBurningFinished() const
{
    return isBurning() ? pMachineBurning_->isBurningFinished() : false;
}

/* End MCDMGDAT.CPP *************************************************/
