/*
 * E N T R A N C E . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "system/pathname.hpp"
#include "world4d/soundman.hpp"
#include "sim/manager.hpp"

#include "machphys/entrance.hpp"
#include "machphys/door.hpp"
#include "machphys/snddata.hpp"

//////////////////////////////////////////////////////////////////////////////////////////

MachPhysEntrance::MachPhysEntrance()
    : pDoor1_(nullptr)
    , pDoor2_(nullptr)
    , openCount_(0)
{

    TEST_INVARIANT;
}
//////////////////////////////////////////////////////////////////////////////////////////

MachPhysEntrance::~MachPhysEntrance()
{
    TEST_INVARIANT;

    delete pDoor1_;
    delete pDoor2_;
}
//////////////////////////////////////////////////////////////////////////////////////////

void MachPhysEntrance::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}
//////////////////////////////////////////////////////////////////////////////////////////

void MachPhysEntrance::addDoor(
    W4dEntity* pPhysDoor,
    const MexVec3& displacement,
    MATHEX_SCALAR speed,
    MATHEX_SCALAR acceleration)
{
    PRE(nDoors() < 2);

    // Create a new door
    MachPhysDoor* pDoor = new MachPhysDoor(pPhysDoor, displacement, speed, acceleration);
    if (pDoor1_ == nullptr)
        pDoor1_ = pDoor;
    else
        pDoor2_ = pDoor;
}
//////////////////////////////////////////////////////////////////////////////////////////

uint MachPhysEntrance::nDoors() const
{
    uint count = 0;
    if (pDoor1_ != nullptr)
    {
        ++count;
        if (pDoor2_ != nullptr)
            ++count;
    }

    return count;
}
//////////////////////////////////////////////////////////////////////////////////////////

PhysRelativeTime MachPhysEntrance::isOpen(bool doOpen)
{
    PhysRelativeTime interval = 0;

    size_t oldOpenCount = openCount_;

    if (doOpen)
        ++openCount_;
    else if (openCount_ > 0)
        --openCount_;

    bool changeState = (openCount_ == 1 and doOpen) or (openCount_ == 0 and oldOpenCount != 0);

    // Check changing state
    if (changeState)
    {
        // DevSound::instance().playSampleFromMemory( "sounds/move05.wav" );

        if (pDoor1_ != nullptr)
        {
            W4dSoundManager::instance()
                .play(&(pDoor1_->doorEntity()), SID_DOOR, SimManager::instance().currentTime(), 1);
            //            W4dSoundManager::instance().play( &(pDoor1_->doorEntity()), SysPathName( "sounds/move05.wav"
            //            ),
            //                                          SimManager::instance().currentTime(), 100.0, 30.0,
            //                                          W4dSoundManager::PLAY_ONCE );
        }

        // Initiate animation for the doors
        if (pDoor1_ != nullptr)
        {
            PhysRelativeTime duration = pDoor1_->changeState(openCount_ == 1);
            if (duration > interval)
                interval = duration;
        }

        if (pDoor2_ != nullptr)
        {
            PhysRelativeTime duration = pDoor2_->changeState(openCount_ == 1);
            if (duration > interval)
                interval = duration;
        }
    }

    return interval;
}
//////////////////////////////////////////////////////////////////////////////////////////

bool MachPhysEntrance::isOpen() const
{
    return openCount_ > 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
/* End ENTRANCE.CPP *************************************************/
