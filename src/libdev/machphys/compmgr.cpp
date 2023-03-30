/*
 * C O M P M G R . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include <algorithm>

#include "envirnmt/planet.hpp"
#include "machphys/compmgr.hpp"
#include "machphys/compitem.hpp"
#include "machphys/plansurf.hpp"

#include "world4d/manager.hpp"
#include "world4d/scenemgr.hpp"

//
// Ids associated with Complexity items are hard coded
// ids from 10000 to 10199 are reserved for this class
// 10000-10019 boolean items
// 10020-10039 choice items
//
// Correspondance table:
//
//  10000   dynamic lights
//  10001   vapour trails
//  10020   sky
//

// static
MachPhysComplexityManager& MachPhysComplexityManager::instance()
{
    static MachPhysComplexityManager instance_;
    return instance_;
}

MachPhysComplexityManager::MachPhysComplexityManager()
    : vapourTrailsEnabled_(true)
{
    static constexpr ItemId dynamicLightsId = 10000;
    static constexpr ItemId vapourTrailsId = 10001;
    static constexpr ItemId skyId = 10020;

    auto* dynamicLight = new MachPhysComplexityBooleanItem(dynamicLightsId, true);
    dynamicLight->setCallback([dynamicLight](MachPhysComplexityItem*) {
        if (W4dManager::instance().hasSceneManager())
        {
            W4dSceneManager* manager = W4dManager::instance().sceneManager();
            manager->dynamicLightsEnabled(dynamicLight->enabled());
        }
    });

    auto* vapourTrails = new MachPhysComplexityBooleanItem(vapourTrailsId, vapourTrailsEnabled_);
    vapourTrails->setCallback([vapourTrails](MachPhysComplexityItem*) {
        MachPhysComplexityManager::instance().vapourTrailsEnabled(vapourTrails->enabled());
    });

    booleanItems_ = {
        dynamicLight,
        vapourTrails,
    };

    auto* skyComplexityItem = new MachPhysComplexityChoiceItem(skyId, 3, 2);
    skyComplexityItem->setCallback([skyComplexityItem](MachPhysComplexityItem*) {
        MachPhysPlanetSurface* pSurface = MachPhysComplexityManager::instance().planetSurface();
        if (pSurface && pSurface->hasEnvironment())
        {
            uint choice = skyComplexityItem->choice();
            EnvPlanetEnvironment::VisibilityLevel vLevel = EnvPlanetEnvironment::NO_SKY;
            switch (choice)
            {
                case 0:
                    vLevel = EnvPlanetEnvironment::NO_SKY;
                    break;
                case 1:
                    vLevel = EnvPlanetEnvironment::SKY_WO_SATELLITES;
                    break;
                case 2:
                    vLevel = EnvPlanetEnvironment::FULL_SKY;
                    break;
                default:
                    ASSERT_BAD_CASE_INFO(choice);
            }
            pSurface->environment().setVisibilityLevel(vLevel);
        }
    });

    choiceItems_ = {
        skyComplexityItem,
    };

    TEST_INVARIANT;
}

MachPhysComplexityManager::~MachPhysComplexityManager()
{
    TEST_INVARIANT;

    for (BooleanItems::iterator it = booleanItems_.begin(); it != booleanItems_.end(); ++it)
        _DELETE((*it));

    for (ChoiceItems::iterator it = choiceItems_.begin(); it != choiceItems_.end(); ++it)
        _DELETE((*it));
}

#ifndef NDEBUG
static bool validId(uint id)
{
    return (id >= 10000 and id <= 10199);
}
#endif

const MachPhysComplexityManager::BooleanItems& MachPhysComplexityManager::booleanItems() const
{
    return booleanItems_;
}
const MachPhysComplexityManager::ChoiceItems& MachPhysComplexityManager::choiceItems() const
{
    return choiceItems_;
}

void MachPhysComplexityManager::changeBooleanItem(const uint& id, bool enabled)
{
    ASSERT_INFO(id);
    PRE(validId(id));

    bool idFound = false;
    for (BooleanItems::iterator it = booleanItems_.begin(); it != booleanItems_.end() and not idFound; ++it)
    {
        MachPhysComplexityBooleanItem* pItem = *it;
        if (pItem->id() == id)
        {
            pItem->changeState(enabled);
            pItem->update();
            idFound = true;
        }
    }
    ASSERT(idFound, "id not found");
}

void MachPhysComplexityManager::changeChoiceItem(const uint& id, uint choice)
{
    ASSERT_INFO(id);
    PRE(validId(id));

    bool idFound = false;
    for (ChoiceItems::iterator it = choiceItems_.begin(); it != choiceItems_.end() and not idFound; ++it)
    {
        MachPhysComplexityChoiceItem* pItem = *it;
        if (pItem->id() == id)
        {
            pItem->changeState(choice);
            pItem->update();
            idFound = true;
        }
    }
    ASSERT(idFound, "id not found");
}

// static
static void updateItem(MachPhysComplexityItem* item)
{
    item->update();
}

void MachPhysComplexityManager::updateSceneParameters()
{
    for_each(booleanItems_.begin(), booleanItems_.end(), updateItem);
    for_each(choiceItems_.begin(), choiceItems_.end(), updateItem);
}

bool MachPhysComplexityManager::hasPlanetSurface() const
{
    return pPlanetSurface_ != nullptr;
}

void MachPhysComplexityManager::planetSurface(MachPhysPlanetSurface* planetSurface)
{
    pPlanetSurface_ = planetSurface;
}

MachPhysPlanetSurface* MachPhysComplexityManager::planetSurface()
{
    return pPlanetSurface_;
}

bool MachPhysComplexityManager::vapourTrailsEnabled() const
{
    return vapourTrailsEnabled_;
}

void MachPhysComplexityManager::vapourTrailsEnabled(bool enabled)
{
    vapourTrailsEnabled_ = enabled;
}

void MachPhysComplexityManager::CLASS_INVARIANT
{
    INVARIANT(this != nullptr);
}

std::ostream& operator<<(std::ostream& o, const MachPhysComplexityManager& t)
{

    o << "MachPhysComplexityManager " << (void*)&t << " start" << std::endl;
    o << "MachPhysComplexityManager " << (void*)&t << " end" << std::endl;

    return o;
}

/* End COMPMGR.CPP **************************************************/
