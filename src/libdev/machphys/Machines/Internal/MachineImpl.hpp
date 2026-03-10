/*
 * M A C H I N E I . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysMachineImpl

    Implementation class for MachPhysMachine
*/

#ifndef _MACHPHYS_MACHINEI_HPP
#define _MACHPHYS_MACHINEI_HPP

#include "base/base.hpp"
#include "base/Persistence.hpp"
#include "ctl/CountedPtr.hpp"
#include "mathex/mathex.hpp"
#include "mathex/Radians.hpp"
#include "world4d/Entity/Composite.hpp"
#include "world4d/Entity/Link.hpp"
#include "machphys/Machines/MachineExplosionData.hpp"
#include "machphys/machphys.hpp"
#include "machphys/Terrain/PlanetSurface.hpp"
#include "machphys/Machines/Machine.hpp"

#include "ctl/Vector.hpp"

class W4dLink;
class MachPhysLocomotionMethod;
class MachPhysMachineDamageData;
class MachPhysMachineExplosion;
class MachPhysTurnerTracker;
class MachPhysMachineData;

class MachPhysMachineImpl
// Canonical form revoked
{
public:
    virtual ~MachPhysMachineImpl();

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT(MachPhysMachineImpl);
    PER_FRIEND_READ_WRITE(MachPhysMachineImpl);

private:
    friend std::ostream& operator<<(std::ostream& o, const MachPhysMachineImpl& t);

    friend class MachPhysMachine;

    // One time constructor
    MachPhysMachineImpl();
    MachPhysMachineImpl(
        const MachPhysMachineImpl& copyMe,
        const MachPhysMachineData& data,
        MachPhys::Race race,
        size_t bodyLevel,
        size_t brainLevel);
    MachPhysMachineImpl(const MachPhysMachineImpl&);
    MachPhysMachineImpl& operator=(const MachPhysMachineImpl&);

    // data members
    MATHEX_SCALAR maxTranslationSpeed_; // Metres / second
    MexRadians maxRotationSpeed_; // Radians / second
    MATHEX_SCALAR maxTranslationAcceleration_; // Metres / second / second
    MexRadians maxRotationAcceleration_; // Radians / second / second

    MachPhysLocomotionMethod* pLocomotionMethod_;
    MachPhysMachineDamageData* pDamageData_;

    W4dLink* pFaceplate_;
    W4dComposite::W4dLinks shakeLinks_; // The links to be used for a shaking animation

    MachPhys::Race race_;

    MachPhysMachineExplosion* pExplosion_;

    MachPhysMachineExplosionDataPtr explosionDataPtr_;

    W4dLink* pUpperBodyTurnLink_; // Link used to turn upper body. NULL if can't turn.
    MachPhysTurnerTracker* pTurnerTracker_; // Controls turning and tracking

    size_t bodyLevel_; // Hardware Level
    size_t brainLevel_; // Software Level

    MachPhysPlanetSurface::Floors floors_;

    MachPhysMachine::ControlType machineControlType_;
};

PER_DECLARE_PERSISTENT(MachPhysMachineImpl);

#endif

/* End MACHINEI.HPP *************************************************/
