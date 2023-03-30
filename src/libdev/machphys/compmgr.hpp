/*
 * C O M P M G R . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysComplexityManager

    A brief description of the class should go in here
*/

#ifndef _COMPMGR_HPP
#define _COMPMGR_HPP

#include "base/base.hpp"

#include <vector>

using ItemId = uint;

class MachPhysComplexityItem;
class MachPhysComplexityBooleanItem;
class MachPhysComplexityChoiceItem;
class MachPhysPlanetSurface;

class MachPhysComplexityManager
// Canonical form revoked
{
public:
    //  Singleton class
    static MachPhysComplexityManager& instance();
    ~MachPhysComplexityManager();

    void CLASS_INVARIANT;

    using BooleanItems = std::vector<MachPhysComplexityBooleanItem*>;
    using ChoiceItems = std::vector<MachPhysComplexityChoiceItem*>;

    const BooleanItems& booleanItems() const;
    const ChoiceItems& choiceItems() const;

    void changeBooleanItem(const ItemId& id, bool enabled);
    void changeChoiceItem(const ItemId& id, uint choice);

    // update the scene rendering parameters of items having their changed_ flag set to true
    void updateSceneParameters();

    // planet surface
    bool hasPlanetSurface() const;
    void planetSurface(MachPhysPlanetSurface* pSurface);
    MachPhysPlanetSurface* planetSurface();

    void vapourTrailsEnabled(bool);
    bool vapourTrailsEnabled() const;

private:
    friend std::ostream& operator<<(std::ostream& o, const MachPhysComplexityManager& t);

    MachPhysComplexityManager(const MachPhysComplexityManager&);
    MachPhysComplexityManager& operator=(const MachPhysComplexityManager&);

    MachPhysComplexityManager();

    BooleanItems booleanItems_;
    ChoiceItems choiceItems_;

    MachPhysPlanetSurface* pPlanetSurface_;
    bool vapourTrailsEnabled_;
};

#endif

/* End COMPMGR.HPP **************************************************/
