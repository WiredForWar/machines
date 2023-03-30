/*
 * M A C H V M A N . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

// #include "machlog/internal/VMinfo.hpp"
// #include "machlog/internal/VMail.hpp"

#include "machlog/machvman.hpp"

#include "stdlib/string.hpp"

#include "machlog/actor.hpp"
#include "machlog/aggressr.hpp"
#include "machlog/construc.hpp"
#include "machlog/machine.hpp"
#include "machlog/races.hpp"
#include "machlog/rescarr.hpp"
#include "machlog/technici.hpp"
#include "machlog/vmman.hpp"
#include "machlog/RecentEventsManager.hpp"

#include "machphys/random.hpp"

// static
MachLogMachineVoiceMailManager& MachLogMachineVoiceMailManager::instance()
{
    static MachLogMachineVoiceMailManager instance_;
    return instance_;
}

MachLogMachineVoiceMailManager::MachLogMachineVoiceMailManager()
{
    TEST_INVARIANT;
}

MachLogMachineVoiceMailManager::~MachLogMachineVoiceMailManager()
{
    TEST_INVARIANT;
}

void MachLogMachineVoiceMailManager::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

void MachLogMachineVoiceMailManager::postNewMail(const MachActor& fromActor, MachineVoiceMailEventID id)
{
    if (fromActor.isIn1stPersonView())
        return;

    VoiceMailID globalId = getGlobalFromMachineEvent(fromActor.objectType(), fromActor.subType(), id, fromActor.id());
    bool posted = MachLogVoiceMailManager::instance().postNewMail(globalId, fromActor.id(), fromActor.race());
    if (posted)
    {
        MachLogRecentEventsManager::instance().onVoiceMailPosted(fromActor, id);
    }
}

std::ostream& operator<<(std::ostream& o, const MachLogMachineVoiceMailManager& t)
{

    o << "MachLogMachineVoiceMailManager " << (void*)&t << " start" << std::endl;
    o << "MachLogMachineVoiceMailManager " << (void*)&t << " end" << std::endl;

    return o;
}

std::ostream& operator<<(std::ostream& o, MachineVoiceMailEventID id)
{

    switch (id)
    {
        case MachineVoiceMailEventID::SELF_DESTRUCT:
            o << "MEV_SELF_DESTRUCT";
            break;
        case MachineVoiceMailEventID::BUILT:
            o << "MEV_BUILT";
            break;
        case MachineVoiceMailEventID::DESTROYED:
            o << "MEV_DESTROYED";
            break;
        case MachineVoiceMailEventID::NEAR_DEATH:
            o << "MEV_NEAR_DEATH";
            break;
        case MachineVoiceMailEventID::CHANGED_RACE:
            o << "MEV_CHANGED_RACE";
            break;
        case MachineVoiceMailEventID::VIRUS_INFECTED:
            o << "MEV_VIRUS_INFECTED";
            break;
        case MachineVoiceMailEventID::DAMAGED:
            o << "MEV_DAMAGED";
            break;
        case MachineVoiceMailEventID::TASKED:
            o << "MEV_TASKED";
            break;
        case MachineVoiceMailEventID::MOVING:
            o << "MEV_MOVING";
            break;
        case MachineVoiceMailEventID::SELECTED:
            o << "MEV_SELECTED";
            break;
        case MachineVoiceMailEventID::TARGET_ENEMY:
            o << "MEV_TARGET_ENEMY";
            break;
        case MachineVoiceMailEventID::RECYCLE:
            o << "MEV_RECYCLE";
            break;
        case MachineVoiceMailEventID::HEAL_TARGET:
            o << "MEV_HEAL_TARGET";
            break;
        case MachineVoiceMailEventID::HEALING_COMPLETE:
            o << "MEV_HEALING_COMPLETE";
            break;
        case MachineVoiceMailEventID::LAUNCH_VIRUS:
            o << "MEV_LAUNCH_VIRUS";
            break;
        case MachineVoiceMailEventID::VIRUS_LAUNCHED:
            o << "MEV_VIRUS_LAUNCHED";
            break;
        case MachineVoiceMailEventID::TREACHERY_TARGET:
            o << "MEV_TREACHERY_TARGET";
            break;
        case MachineVoiceMailEventID::BUILDING_COMPLETE:
            o << "MEV_BUILDING_COMPLETE";
            break;
        case MachineVoiceMailEventID::AWAITING_NEW_JOB:
            o << "MEV_AWAITING_NEW_JOB";
            break;
        case MachineVoiceMailEventID::MOVING_TO_NEXT:
            o << "MEV_MOVING_TO_NEXT";
            break;
        case MachineVoiceMailEventID::MOVE_TO_SITE:
            o << "MEV_MOVE_TO_SITE";
            break;
        case MachineVoiceMailEventID::BUILDING_CAPTURED:
            o << "MEV_BUILDING_CAPTURED";
            break;
        case MachineVoiceMailEventID::BUILDING_DECONSTRUCTED:
            o << "MEV_BUILDING_DECONSTRUCTED";
            break;
        case MachineVoiceMailEventID::BUILDING_REPAIRED:
            o << "MEV_BUILDING_REPAIRED";
            break;
        case MachineVoiceMailEventID::MINERAL_LOCATED:
            o << "MEV_MINERAL_LOCATED";
            break;
        case MachineVoiceMailEventID::SEARCH_COMPLETE:
            o << "MEV_SEARCH_COMPLETE";
            break;

            DEFAULT_ASSERT_BAD_CASE(id);
    }

    return o;
}

VoiceMailID MachLogMachineVoiceMailManager::getGlobalFromMachineEvent(
    MachLog::ObjectType ot,
    int subType,
    MachineVoiceMailEventID id,
    UtlId actorId)
{

    // this method has multiple return points to avoid writing 192 break; statements.

    ASSERT_INFO(ot);

    ASSERT(
        ot == MachLog::AGGRESSOR or ot == MachLog::ADMINISTRATOR or ot == MachLog::CONSTRUCTOR
            or ot == MachLog::GEO_LOCATOR or ot == MachLog::SPY_LOCATOR or ot == MachLog::TECHNICIAN
            or ot == MachLog::RESOURCE_CARRIER or ot == MachLog::APC,
        "No, that's rubbish. Not a recognised machine type.");

    switch (ot)
    {
        // ========================================== A G G R E S S O R S ==========================================
        case MachLog::AGGRESSOR:
            switch (subType)
            {
                //--------------------------------------- G R U N T ---------------------------------------
                case MachPhys::GRUNT:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_GRUNT_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_GRUNT_BUILT0;
                        case MachineVoiceMailEventID::DESTROYED:
                            return VID_GRUNT_DESTROYED;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_GRUNT_NEAR_DEATH0;
                                case 1:
                                    return VID_GRUNT_NEAR_DEATH1;
                                case 2:
                                    return VID_GRUNT_NEAR_DEATH2;
                                case 3:
                                    return VID_GRUNT_NEAR_DEATH3;
                            }
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_GRUNT_CHANGED_RACE0;
                                case 1:
                                    return VID_GRUNT_CHANGED_RACE1;
                            }
                        case MachineVoiceMailEventID::VIRUS_INFECTED:
                            return VID_GRUNT_VIRUS_INFECTED;
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_GRUNT_DAMAGED;
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_GRUNT_TASKED0;
                                case 1:
                                    return VID_GRUNT_TASKED1;
                                case 2:
                                    return VID_GRUNT_TASKED2;
                                case 3:
                                    return VID_GRUNT_TASKED3;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_GRUNT_MOVING0;
                                case 1:
                                    return VID_GRUNT_MOVING1;
                                case 2:
                                    return VID_GRUNT_MOVING2;
                                case 3:
                                    return VID_GRUNT_MOVING3;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_GRUNT_SELECTED0;
                                case 1:
                                    return VID_GRUNT_SELECTED1;
                                case 2:
                                    return VID_GRUNT_SELECTED2;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 5))
                            {
                                case 0:
                                    return VID_GRUNT_TARGET_ENEMY0;
                                case 1:
                                    return VID_GRUNT_TARGET_ENEMY1;
                                case 2:
                                    return VID_GRUNT_TARGET_ENEMY2;
                                case 3:
                                    return VID_GRUNT_TARGET_ENEMY3;
                                case 4:
                                    return VID_GRUNT_TARGET_ENEMY4;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                            return VID_GRUNT_RECYCLE;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                //----------------------------------- A S S A S S I N -------------------------------------
                case MachPhys::ASSASSIN:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_ASSASSIN_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_ASSASSIN_BUILT;
                        case MachineVoiceMailEventID::DESTROYED:
                            return VID_ASSASSIN_DESTROYED;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_ASSASSIN_NEAR_DEATH0;
                                case 1:
                                    return VID_ASSASSIN_NEAR_DEATH1;
                            }
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_ASSASSIN_CHANGED_RACE0;
                                case 1:
                                    return VID_ASSASSIN_CHANGED_RACE1;
                            }
                        case MachineVoiceMailEventID::VIRUS_INFECTED:
                            return VID_ASSASSIN_VIRUS_INFECTED;
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_ASSASSIN_DAMAGED;
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_ASSASSIN_TASKED0;
                                case 1:
                                    return VID_ASSASSIN_TASKED1;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_ASSASSIN_MOVING0;
                                case 1:
                                    return VID_ASSASSIN_MOVING1;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_ASSASSIN_SELECTED0;
                                case 1:
                                    return VID_ASSASSIN_SELECTED1;
                                case 2:
                                    return VID_ASSASSIN_SELECTED2;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_ASSASSIN_TARGET_ENEMY0;
                                case 1:
                                    return VID_ASSASSIN_TARGET_ENEMY1;
                                case 2:
                                    return VID_ASSASSIN_TARGET_ENEMY2;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                            return VID_ASSASSIN_RECYCLE;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                //----------------------------------- K N I G H T ---------------------------------------
                case MachPhys::KNIGHT:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_KNIGHT_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_KNIGHT_BUILT;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            return VID_KNIGHT_NEAR_DEATH;
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            return VID_KNIGHT_CHANGED_RACE;
                        case MachineVoiceMailEventID::DAMAGED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_KNIGHT_DAMAGED0;
                                case 1:
                                    return VID_KNIGHT_DAMAGED1;
                            }
                        case MachineVoiceMailEventID::TASKED:
                        case MachineVoiceMailEventID::RECYCLE:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_KNIGHT_TASKED0;
                                case 1:
                                    return VID_KNIGHT_TASKED1;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_KNIGHT_MOVING0;
                                case 1:
                                    return VID_KNIGHT_MOVING1;
                                case 2:
                                    return VID_KNIGHT_MOVING2;
                                case 3:
                                    return VID_KNIGHT_MOVING3;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_KNIGHT_SELECTED0;
                                case 1:
                                    return VID_KNIGHT_SELECTED1;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_KNIGHT_TARGET_ENEMY0;
                                case 1:
                                    return VID_KNIGHT_TARGET_ENEMY1;
                            }

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                //----------------------------------- B A L L I S T A ---------------------------------------
                case MachPhys::BALLISTA:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_BALLISTA_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            {
                                ASSERT(actorId != 0, "Wasn't passed an actor id!");
                                ASSERT(MachLogRaces::instance().actorExists(actorId), "That actor doesn't exist!");

                                const MachActor& actor = MachLogRaces::instance().actor(actorId);
                                ASSERT(
                                    actor.objectType() == MachLog::AGGRESSOR,
                                    "Id of 'aggressor' was for a non-aggressor actor!");

                                const MachLogAggressor& ballista = actor.asAggressor();
                                if (ballista.hwLevel() == 3)
                                {
                                    // either a crusader or an eradicator
                                    if (ballista.weaponCombo() == MachPhys::T_MULTI_LAUNCHER1)
                                        return VID_BALLISTA_CRUSADER_BUILT;
                                    else
                                        return VID_BALLISTA_ERADICATOR_BUILT;
                                }
                                else
                                {
                                    // must be a goliath
                                    return VID_BALLISTA_GOLIATH_BUILT;
                                }
                            }
                        case MachineVoiceMailEventID::DESTROYED:
                            return VID_BALLISTA_DESTROYED;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            return VID_BALLISTA_NEAR_DEATH;
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BALLISTA_CHANGED_RACE0;
                                case 1:
                                    return VID_BALLISTA_CHANGED_RACE1;
                            }
                        case MachineVoiceMailEventID::VIRUS_INFECTED:
                            return VID_BALLISTA_VIRUS_INFECTED;
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_BALLISTA_DAMAGED;
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_BALLISTA_TASKED0;
                                case 1:
                                    return VID_BALLISTA_TASKED1;
                                case 2:
                                    return VID_BALLISTA_TASKED2;
                                case 3:
                                    return VID_BALLISTA_TASKED3;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_BALLISTA_MOVING0;
                                case 1:
                                    return VID_BALLISTA_MOVING1;
                                case 2:
                                    return VID_BALLISTA_MOVING2;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BALLISTA_SELECTED0;
                                case 1:
                                    return VID_BALLISTA_SELECTED1;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_BALLISTA_TARGET_ENEMY0;
                                case 1:
                                    return VID_BALLISTA_TARGET_ENEMY1;
                                case 2:
                                    return VID_BALLISTA_TARGET_ENEMY2;
                                case 3:
                                    return VID_BALLISTA_TARGET_ENEMY3;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                            return VID_BALLISTA_RECYCLE;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }

                //----------------------------------- N I N J A ---------------------------------------
                case MachPhys::NINJA:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_NINJA_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_NINJA_BUILT;
                        case MachineVoiceMailEventID::DESTROYED:
                            return VID_NINJA_DESTROYED;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_NINJA_NEAR_DEATH0;
                                case 1:
                                    return VID_NINJA_NEAR_DEATH1;
                            }
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            return VID_NINJA_CHANGED_RACE;
                        case MachineVoiceMailEventID::VIRUS_INFECTED:
                            break;
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_NINJA_DAMAGED;
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_NINJA_TASKED0;
                                case 1:
                                    return VID_NINJA_TASKED1;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_NINJA_MOVING0;
                                case 1:
                                    return VID_NINJA_MOVING1;
                                case 2:
                                    return VID_NINJA_MOVING2;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 3))
                            {
                                case 0:
                                    return VID_NINJA_SELECTED0;
                                case 1:
                                    return VID_NINJA_SELECTED1;
                                case 2:
                                    return VID_NINJA_SELECTED2;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_NINJA_TARGET_ENEMY0;
                                case 1:
                                    return VID_NINJA_TARGET_ENEMY1;
                                case 2:
                                    return VID_NINJA_TARGET_ENEMY2;
                                case 3:
                                    return VID_NINJA_TARGET_ENEMY3;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                            return VID_NINJA_RECYCLE;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                    DEFAULT_ASSERT_BAD_CASE(subType);
            }
        // ====================================== A D M I N I S T R A T O R S ======================================
        case MachLog::ADMINISTRATOR:
            switch (subType)
            {
                //--------------------------------------- B O S S ---------------------------------------
                case MachPhys::BOSS:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_BOSS_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            {
                                ASSERT(actorId != 0, "Wasn't passed an actor id!");
                                ASSERT(MachLogRaces::instance().actorExists(actorId), "That actor doesn't exist!");

                                const MachActor& actor = MachLogRaces::instance().actor(actorId);
                                ASSERT(actor.objectIsMachine(), "Id of 'boss' was for a non-machine actor!");

                                if (actor.asMachine().hwLevel() == 2)
                                    return VID_BOSS_BEE_BUILT;
                                else
                                    return VID_BOSS_WASP_BUILT;
                            }
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BOSS_NEAR_DEATH0;
                                case 1:
                                    return VID_BOSS_NEAR_DEATH1;
                            }
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            return VID_BOSS_CHANGED_RACE;
                        case MachineVoiceMailEventID::DAMAGED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BOSS_DAMAGED0;
                                case 1:
                                    return VID_BOSS_DAMAGED1;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_BOSS_TASKED0;
                                case 1:
                                    return VID_BOSS_TASKED1;
                                case 2:
                                    return VID_BOSS_TASKED2;
                                case 3:
                                    return VID_BOSS_TASKED3;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 4))
                            {
                                case 0:
                                    return VID_BOSS_MOVING0;
                                case 1:
                                    return VID_BOSS_MOVING1;
                                case 2:
                                    return VID_BOSS_MOVING2;
                                case 3:
                                    return VID_BOSS_MOVING3;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BOSS_SELECTED0;
                                case 1:
                                    return VID_BOSS_SELECTED1;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_BOSS_TARGET_ENEMY0;
                                case 1:
                                    return VID_BOSS_TARGET_ENEMY1;
                            }

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                //----------------------------------- O V E R S E E R -------------------------------------
                case MachPhys::OVERSEER:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_OVERSEER_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_OVERSEER_BUILT;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            return VID_OVERSEER_NEAR_DEATH;
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_OVERSEER_CHANGED_RACE0;
                                case 1:
                                    return VID_OVERSEER_CHANGED_RACE1;
                            }
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_OVERSEER_DAMAGED;
                        case MachineVoiceMailEventID::TASKED:
                            return VID_OVERSEER_TASKED;
                        case MachineVoiceMailEventID::MOVING:
                            return VID_OVERSEER_MOVING;
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_OVERSEER_SELECTED0;
                                case 1:
                                    return VID_OVERSEER_SELECTED1;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_OVERSEER_TARGET_ENEMY0;
                                case 1:
                                    return VID_OVERSEER_TARGET_ENEMY1;
                            }
                        case MachineVoiceMailEventID::RECYCLE:
                            return VID_OVERSEER_MOVING;
                        case MachineVoiceMailEventID::HEAL_TARGET:
                            return VID_OVERSEER_HEAL_TARGET;
                        case MachineVoiceMailEventID::HEALING_COMPLETE:
                            return VID_OVERSEER_HEALING_COMPLETE;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                //----------------------------------- C O M M A N D E R -------------------------------------
                case MachPhys::COMMANDER:
                    switch (id)
                    {
                        case MachineVoiceMailEventID::SELF_DESTRUCT:
                            return VID_COMMANDER_SELF_DESTRUCT;
                        case MachineVoiceMailEventID::BUILT:
                            return VID_COMMANDER_BUILT;
                        case MachineVoiceMailEventID::NEAR_DEATH:
                            return VID_COMMANDER_NEAR_DEATH;
                        case MachineVoiceMailEventID::CHANGED_RACE:
                            return VID_COMMANDER_CHANGED_RACE;
                        case MachineVoiceMailEventID::DAMAGED:
                            return VID_COMMANDER_DAMAGED;
                        case MachineVoiceMailEventID::RECYCLE:
                        case MachineVoiceMailEventID::TASKED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_COMMANDER_TASKED0;
                                case 1:
                                    return VID_COMMANDER_TASKED1;
                            }
                        case MachineVoiceMailEventID::MOVING:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_COMMANDER_MOVING0;
                                case 1:
                                    return VID_COMMANDER_MOVING1;
                            }
                        case MachineVoiceMailEventID::SELECTED:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_COMMANDER_SELECTED0;
                                case 1:
                                    return VID_COMMANDER_SELECTED1;
                            }
                        case MachineVoiceMailEventID::TARGET_ENEMY:
                            switch (MachPhysRandom::randomInt(0, 2))
                            {
                                case 0:
                                    return VID_COMMANDER_TARGET_ENEMY0;
                                case 1:
                                    return VID_COMMANDER_TARGET_ENEMY1;
                            }

                        case MachineVoiceMailEventID::HEAL_TARGET:
                            return VID_COMMANDER_HEAL_TARGET;
                        case MachineVoiceMailEventID::HEALING_COMPLETE:
                            return VID_COMMANDER_HEALING_COMPLETE;
                        case MachineVoiceMailEventID::TREACHERY_TARGET:
                            return VID_COMMANDER_TREACHERY_TARGET;

                            DEFAULT_ASSERT_BAD_CASE(id);
                    }
                    DEFAULT_ASSERT_BAD_CASE(subType);
            }
        // ====================================== T E C H N I C I A N ======================================
        case MachLog::TECHNICIAN:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_TECHNICIAN_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    {
                        ASSERT(actorId != 0, "Wasn't passed an actor id!");
                        ASSERT(MachLogRaces::instance().actorExists(actorId), "That actor doesn't exist!");

                        const MachActor& actor = MachLogRaces::instance().actor(actorId);
                        ASSERT(
                            actor.objectType() == MachLog::TECHNICIAN,
                            "Id of 'technician' was for a non-technician actor!");

                        switch (actor.asTechnician().subType())
                        {
                            case MachPhys::LAB_TECH:
                                return VID_TECHNICIAN_BUILT;
                            case MachPhys::TECH_BOY:
                                return VID_TECHNICIAN_ENGINEER_BUILT;
                            case MachPhys::BRAIN_BOX:
                                return VID_TECHNICIAN_BRAINBOX_BUILT;

                                DEFAULT_ASSERT_BAD_CASE(actor.asTechnician().subType());
                        }
                    }
                case MachineVoiceMailEventID::DESTROYED:
                    return VID_TECHNICIAN_DESTROYED;
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_TECHNICIAN_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_TECHNICIAN_CHANGED_RACE0;
                        case 1:
                            return VID_TECHNICIAN_CHANGED_RACE1;
                    }
                case MachineVoiceMailEventID::VIRUS_INFECTED:
                    return VID_TECHNICIAN_VIRUS_INFECTED;
                case MachineVoiceMailEventID::DAMAGED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_TECHNICIAN_DAMAGED;
                        case 1:
                            return VID_TECHNICIAN_DESTROYED;
                    }
                case MachineVoiceMailEventID::TASKED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_TECHNICIAN_TASKED0;
                        case 1:
                            return VID_TECHNICIAN_TASKED1;
                    }
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_TECHNICIAN_MOVING0;
                        case 1:
                            return VID_TECHNICIAN_MOVING1;
                        case 2:
                            return VID_TECHNICIAN_MOVING2;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_TECHNICIAN_SELECTED0;
                        case 1:
                            return VID_TECHNICIAN_SELECTED1;
                    }
                case MachineVoiceMailEventID::RECYCLE:
                    return VID_TECHNICIAN_RECYCLE;

                    DEFAULT_ASSERT_BAD_CASE(id);
            }
        // ==================================== G E O - L O C A T O R =======================================
        case MachLog::GEO_LOCATOR:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_GEO_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    return VID_GEO_BUILT;
                case MachineVoiceMailEventID::DESTROYED:
                    return VID_GEO_DESTROYED;
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_GEO_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_GEO_CHANGED_RACE0;
                        case 1:
                            return VID_GEO_CHANGED_RACE1;
                    }
                case MachineVoiceMailEventID::VIRUS_INFECTED:
                    return VID_GEO_VIRUS_INFECTED;
                case MachineVoiceMailEventID::DAMAGED:
                    return VID_GEO_DAMAGED;
                case MachineVoiceMailEventID::TASKED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_GEO_TASKED0;
                        case 1:
                            return VID_GEO_TASKED1;
                    }
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_GEO_MOVING0;
                        case 1:
                            return VID_GEO_MOVING1;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_GEO_SELECTED0;
                        case 1:
                            return VID_GEO_SELECTED1;
                    }
                case MachineVoiceMailEventID::RECYCLE:
                    return VID_GEO_RECYCLE;
                case MachineVoiceMailEventID::MINERAL_LOCATED:
                    return VID_GEO_MINERAL_LOCATED;
                case MachineVoiceMailEventID::SEARCH_COMPLETE:
                    return VID_GEO_SEARCH_COMPLETE;

                    DEFAULT_ASSERT_BAD_CASE(id);
            }
        // ==================================== S P Y - L O C A T O R =======================================
        case MachLog::SPY_LOCATOR:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_SPY_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    return VID_SPY_BUILT;
                case MachineVoiceMailEventID::DESTROYED:
                    return VID_SPY_DESTROYED;
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_SPY_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    return VID_SPY_CHANGED_RACE;
                case MachineVoiceMailEventID::VIRUS_INFECTED:
                    return VID_SPY_VIRUS_INFECTED;
                case MachineVoiceMailEventID::DAMAGED:
                    return VID_SPY_DAMAGED;
                case MachineVoiceMailEventID::TASKED:
                    return VID_SPY_TASKED0;
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_SPY_MOVING0;
                        case 1:
                            return VID_SPY_MOVING1;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    return VID_SPY_SELECTED0;
                case MachineVoiceMailEventID::RECYCLE:
                    return VID_SPY_RECYCLE;

                    DEFAULT_ASSERT_BAD_CASE(id);
            }

        // =================================== C O N S T R U C T O R S ======================================

        // note that all 3 constructors use the same .wavs for all but the "I've been built" voicemails.
        case MachLog::CONSTRUCTOR:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_DOZER_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    switch (subType)
                    {
                        case MachPhys::DOZER:
                            return VID_DOZER_BUILT;
                        case MachPhys::BUILDER:
                            return VID_BUILDER_BUILT;
                        case MachPhys::BEHEMOTH:
                            {
                                ASSERT(actorId != 0, "Wasn't passed an actor id!");
                                ASSERT(MachLogRaces::instance().actorExists(actorId), "That actor doesn't exist!");

                                const MachActor& actor = MachLogRaces::instance().actor(actorId);
                                ASSERT(
                                    actor.objectType() == MachLog::CONSTRUCTOR,
                                    "Id of 'constructor' was for a non-constructor actor!");

                                if (actor.asConstructor().hwLevel() == 4)
                                    return VID_BEHEMOTH_BUILT;
                                else
                                    return VID_TITAN_BUILT;
                            }

                            DEFAULT_ASSERT_BAD_CASE(subType);
                    }
                case MachineVoiceMailEventID::DESTROYED:
                    return VID_DOZER_DESTROYED;
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_DOZER_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_DOZER_CHANGED_RACE0;
                        case 1:
                            return VID_DOZER_CHANGED_RACE1;
                    }
                case MachineVoiceMailEventID::VIRUS_INFECTED:
                    return VID_DOZER_VIRUS_INFECTED;
                case MachineVoiceMailEventID::DAMAGED:
                    return VID_DOZER_DAMAGED;
                case MachineVoiceMailEventID::TASKED:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_DOZER_TASKED0;
                        case 1:
                            return VID_DOZER_TASKED1;
                        case 2:
                            return VID_DOZER_TASKED2;
                    }
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_DOZER_MOVING0;
                        case 1:
                            return VID_DOZER_MOVING1;
                        case 2:
                            return VID_DOZER_MOVING2;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_DOZER_SELECTED0;
                        case 1:
                            return VID_DOZER_SELECTED1;
                    }
                case MachineVoiceMailEventID::RECYCLE:
                    return VID_DOZER_RECYCLE;
                case MachineVoiceMailEventID::BUILDING_COMPLETE:
                    return VID_DOZER_BUILDING_COMPLETE;
                case MachineVoiceMailEventID::AWAITING_NEW_JOB:
                    return VID_DOZER_AWAITING_NEW_JOB;
                case MachineVoiceMailEventID::MOVING_TO_NEXT:
                    return VID_DOZER_MOVING_TO_NEXT;
                case MachineVoiceMailEventID::MOVE_TO_SITE:
                    return VID_DOZER_MOVE_TO_SITE;
                case MachineVoiceMailEventID::BUILDING_CAPTURED:
                    return VID_DOZER_BUILDING_CAPTURED;
                case MachineVoiceMailEventID::BUILDING_DECONSTRUCTED:
                    return VID_DOZER_BUILDING_DECONSTRUCTED;
                case MachineVoiceMailEventID::BUILDING_REPAIRED:
                    return VID_DOZER_BUILDING_REPAIRED;

                    DEFAULT_ASSERT_BAD_CASE(id);
            }
        // ================================ R E S O U R C E    C A R R I E R =======================================
        case MachLog::RESOURCE_CARRIER:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_RESOURCE_CARRIER_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    {
                        ASSERT(actorId != 0, "Wasn't passed an actor id!");
                        ASSERT(MachLogRaces::instance().actorExists(actorId), "That actor doesn't exist!");

                        const MachActor& actor = MachLogRaces::instance().actor(actorId);
                        ASSERT(
                            actor.objectType() == MachLog::RESOURCE_CARRIER,
                            "Id of 'resource carrier' was for a non-resource carrier actor!");

                        if (actor.asResourceCarrier().isScavenger())
                            return VID_RESOURCE_CARRIER_SCAVENGER_BUILT;
                        else
                            return VID_RESOURCE_CARRIER_BUILT;
                    }
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_RESOURCE_CARRIER_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_RESOURCE_CARRIER_CHANGED_RACE0;
                        case 1:
                            return VID_RESOURCE_CARRIER_CHANGED_RACE1;
                    }
                case MachineVoiceMailEventID::DAMAGED:
                    return VID_RESOURCE_CARRIER_DAMAGED;
                case MachineVoiceMailEventID::RECYCLE:
                case MachineVoiceMailEventID::TASKED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_RESOURCE_CARRIER_TASKED0;
                        case 1:
                            return VID_RESOURCE_CARRIER_TASKED1;
                    }
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_RESOURCE_CARRIER_MOVING0;
                        case 1:
                            return VID_RESOURCE_CARRIER_MOVING1;
                        case 2:
                            return VID_RESOURCE_CARRIER_MOVING2;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_RESOURCE_CARRIER_SELECTED0;
                        case 1:
                            return VID_RESOURCE_CARRIER_SELECTED1;
                    }

                    DEFAULT_ASSERT_BAD_CASE(id);
            }
            // ============================================= A P C =============================================
        case MachLog::APC:
            switch (id)
            {
                case MachineVoiceMailEventID::SELF_DESTRUCT:
                    return VID_APC_SELF_DESTRUCT;
                case MachineVoiceMailEventID::BUILT:
                    return VID_APC_BUILT;
                case MachineVoiceMailEventID::DESTROYED:
                    return VID_APC_DESTROYED;
                case MachineVoiceMailEventID::NEAR_DEATH:
                    return VID_APC_NEAR_DEATH;
                case MachineVoiceMailEventID::CHANGED_RACE:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_APC_CHANGED_RACE0;
                        case 1:
                            return VID_APC_CHANGED_RACE1;
                    }
                case MachineVoiceMailEventID::VIRUS_INFECTED:
                    return VID_APC_VIRUS_INFECTED;
                case MachineVoiceMailEventID::DAMAGED:
                    return VID_APC_DAMAGED;
                case MachineVoiceMailEventID::TASKED:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_APC_TASKED0;
                        case 1:
                            return VID_APC_TASKED1;
                        case 2:
                            return VID_APC_TASKED2;
                    }
                case MachineVoiceMailEventID::MOVING:
                    switch (MachPhysRandom::randomInt(0, 3))
                    {
                        case 0:
                            return VID_APC_MOVING0;
                        case 1:
                            return VID_APC_MOVING1;
                        case 2:
                            return VID_APC_MOVING2;
                    }
                case MachineVoiceMailEventID::SELECTED:
                    switch (MachPhysRandom::randomInt(0, 2))
                    {
                        case 0:
                            return VID_APC_SELECTED0;
                        case 1:
                            return VID_APC_SELECTED1;
                    }
                case MachineVoiceMailEventID::RECYCLE:
                    return VID_APC_RECYCLE;

                    DEFAULT_ASSERT_BAD_CASE(id);
            }

        default:
            return VID_BALLISTA_RECYCLE;
    }
}

/* End MACHVMAN.CPP ****************************************************/
