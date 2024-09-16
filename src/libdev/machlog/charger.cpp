/*
 * C H A R G E R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "machphys/charger.hpp"

#include "mathex/transf3d.hpp"
#include "sim/manager.hpp"
#include "machphys/weapon.hpp"
#include "machphys/wepdata.hpp"

#include "machlog/actor.hpp"
#include "machlog/machine.hpp"
#include "machlog/charger.hpp"
#include "machlog/network.hpp"

/* //////////////////////////////////////////////////////////////// */
PER_DEFINE_PERSISTENT(MachLogSuperCharger);

MachLogSuperCharger::MachLogSuperCharger(MachLogRace* pRace, MachPhysSuperCharger* pPhysWeapon, MachActor* pOwner)
    : MachLogWeapon(pRace, pPhysWeapon, pOwner)
    , healing_(false)
    , pPhysSuperCharger_(pPhysWeapon)
{
    HAL_STREAM(
        "MLSuperCharger::CTOR currentlyAttached " << currentlyAttached() << " healing " << healing_ << std::endl);
    HAL_STREAM("(void*)this " << static_cast<const void*>(this) << std::endl);
    TEST_INVARIANT;
}

/* //////////////////////////////////////////////////////////////// */

MachLogSuperCharger::~MachLogSuperCharger()
{
    // just make sure we aren'y currently attached to anything.
    stopAllHealing();
    TEST_INVARIANT;
}

/* //////////////////////////////////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachLogSuperCharger::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void MachLogSuperCharger::setPhysicalSuperCharger(MachPhysSuperCharger* pPhysWeapon)
{
    pPhysSuperCharger_ = pPhysWeapon;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
void MachLogSuperCharger::doFire(MachActor* pTarget, const MachLogFireData&)
{
    HAL_STREAM("MLSuperCharger::doFire at " << pTarget->id() << std::endl);
    HAL_STREAM(
        "MLSuperCharger::doFire currentlyAttached " << currentlyAttached() << " healing " << healing_ << std::endl);

    // it is possible to be calling this code while targettign a non-machine actor in first person. If this
    // is the case, simply do nothing.

    if (pTarget->objectIsMachine())
    {
        if (!healing_)
        {
            // initiate healing; start off by activating the beam animation and adding a heal aura reference

            // register current target of heal.
            target(pTarget);
            // set up observer relationship
            pTarget->attach(this);
            currentlyAttached(true);
            MachLogMachine& mlm = pTarget->asMachine();
            pPhysSuperCharger_->startHealing(mlm.physMachine(), SimManager::instance().currentTime(), 2000);
            mlm.addHealingAuraReference();
            healing_ = true;
            lastUpdateTime_ = SimManager::instance().currentTime();
        }
        else
        {
            // we're already healing - simply add the requisite quantity of hitpoints onto the target.
            // beware rounding errors preventing any points from being added.
            int addAmount = pPhysSuperCharger_->weaponData().damagePoints()
                * (SimManager::instance().currentTime() - lastUpdateTime_);

            if (addAmount != 0)
            {
                if (MachLogNetwork::instance().isNetworkGame())
                {
                    if (MachLogNetwork::instance().remoteStatus(owner().race()) == MachLogNetwork::LOCAL_PROCESS)
                        pTarget->addHitPoints(addAmount);
                }
                else
                    pTarget->addHitPoints(addAmount);

                lastUpdateTime_ = SimManager::instance().currentTime();
            }
        }
    }

    HAL_STREAM(
        "MLSuperCharger::doFire exit currentlyAttached " << currentlyAttached() << " healing " << healing_
                                                         << std::endl);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MachLogSuperCharger::healing() const
{
    return healing_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& o, const MachLogSuperCharger& t)
{

    o << "MachLogSuperCharger " << static_cast<const void*>(&t) << " start" << std::endl;
    o << "MachLogSuperCharger " << static_cast<const void*>(&t) << " end" << std::endl;

    return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

// virtual
bool MachLogSuperCharger::beNotified(W4dSubject* pSubject, W4dSubject::NotificationEvent event, int)
{
    PRE(currentlyAttached() && pSubject == &target());

    bool stayAttached = true;

    switch (event)
    {
        case W4dSubject::DELETED:
            {
                stopAllHealing();
                stayAttached = false;
            }
            break;
        default:;
    }
    return stayAttached;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

MachLogSuperCharger& MachLogSuperCharger::asSuperCharger(MachLogWeapon* pWeapon)
{
    ASSERT_INFO(((const MachLogWeapon*)pWeapon)->physWeapon().type());
    ASSERT(
        ((const MachLogWeapon*)pWeapon)->physWeapon().type() == MachPhys::SUPERCHARGE_ADVANCED
            || ((const MachLogWeapon*)pWeapon)->physWeapon().type() == MachPhys::SUPERCHARGE_SUPER,
        " Wrong weapon type in MLCharger::asSuperCharger\n");
    return *(_STATIC_CAST(MachLogSuperCharger*, pWeapon));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MachLogSuperCharger::stopAllHealing()
{
    // incorporates manual detachment of target as observation subject

    if (currentlyAttached())
    {
        MachLogMachine& mlm = target().asMachine();
        target().detach(this);
        currentlyAttached(false);
        if (healing_)
        {
            HAL_STREAM(" already healing add hitpoints and call stop healing\n");
            int addAmount = pPhysSuperCharger_->weaponData().damagePoints()
                * (SimManager::instance().currentTime() - lastUpdateTime_);
            HAL_STREAM(" damage " << pPhysSuperCharger_->weaponData().damagePoints() << std::endl);
            HAL_STREAM(" adding " << addAmount << " hp\n");
            HAL_STREAM(" calling into mlm\n");
            HAL_STREAM(" id is " << mlm.id() << std::endl);
            mlm.addHitPoints(addAmount);
            mlm.releaseHealingAuraReference();
        }
        HAL_STREAM(" done\n");
    }

    // releases healing aura and sets lastUpdateTime_, if healing was being carried out.

    if (healing_)
    {
        HAL_STREAM(" healing flag is set which means we have a heal healix active\n");
        pPhysSuperCharger_->stopHealing();
        healing_ = false;
        lastUpdateTime_ = SimManager::instance().currentTime();
        HAL_STREAM(" done\n");
    }
    HAL_STREAM("MLSuperCharger::stopAllHealing exit\n");
}

void perWrite(PerOstream& ostr, const MachLogSuperCharger& weapon)
{
    const MachLogWeapon& base1 = weapon;

    ostr << base1;
    ostr << weapon.healing_;
    ostr << weapon.lastUpdateTime_;
}

void perRead(PerIstream& istr, MachLogSuperCharger& weapon)
{
    MachLogWeapon& base1 = weapon;

    istr >> base1;
    istr >> weapon.healing_;
    istr >> weapon.lastUpdateTime_;
}

MachLogSuperCharger::MachLogSuperCharger(PerConstructor con)
    : MachLogWeapon(con)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* End CHARGER.CPP *************************************************/
