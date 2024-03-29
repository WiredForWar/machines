/*
 * A D M I N I S T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysTransporter

    Implement the physical properties of an administrator
*/

#ifndef _ADMINIST_HPP
#define _ADMINIST_HPP

#include "base/base.hpp"
#include "base/persist.hpp"

// #include "machphys/ofactory.hpp"
#include "machphys/machine.hpp"
#include "machphys/attack.hpp"
#include "machphys/machphys.hpp"
// #include "machphys/subid.hpp"

// forward decls
class W4dLink;
class MachPhysMachinePersistence;
class MachPhysAdministratorData;
template <class ID, class PART> class MachPhysObjectFactory;
template <class SUBTYPE> class MachPhysSubTypeId;

class MachPhysAdministrator
    : public MachPhysMachine
    , public MachPhysCanAttack
{
public:
    MachPhysAdministrator(
        W4dEntity* pParent,
        const W4dTransform3d& localTransform,
        MachPhys::AdministratorSubType subType,
        size_t bodyLevel,
        size_t brainLevel,
        MachPhys::Race race,
        MachPhys::WeaponCombo combo);

    ~MachPhysAdministrator() override;

    ///////////////////////////////////////////////////////
    // Inherited from MachPhysCanAttack

    // Makes the underlying physical model available
    W4dComposite& asComposite() override;
    const W4dComposite& asComposite() const override;

    // True iff the attacker can turn its weapon carrier to track a target
    bool canTrackWeaponBase() const override;

    // Override to cause the weapon base to turn in order to track targetObject.
    // Default implementation does nothing.
    void doWeaponBaseTrackTarget(const W4dEntity& targetObject) override;

    // Override to stop the weapon base tracking any target.
    // Default implementation does nothing.
    void doStopWeaponBaseTrackingTarget() override;

    ///////////////////////////////////////////////////////

    const MachPhysMachineData& machineData() const override;

    const MachPhysAdministratorData& data() const;

    MachPhys::AdministratorSubType subType() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysAdministrator& t);

    PER_MEMBER_PERSISTENT(MachPhysAdministrator);
    PER_FRIEND_READ_WRITE(MachPhysAdministrator);

    using Id = MachPhysSubTypeId<MachPhys::AdministratorSubType>;

private:
    // Operation deliberately revoked
    MachPhysAdministrator(const MachPhysAdministrator&);

    // Operation deliberately revoked
    MachPhysAdministrator& operator=(const MachPhysAdministrator&);

    // Operation deliberately revoked
    bool operator==(const MachPhysAdministrator&);

    using Factory = MachPhysObjectFactory<Id, MachPhysAdministrator>;

    //  This is necessary to allow the ti file to instantiate the factory class
    // friend MachPhysAdministrator& Factory::part( const ID& id, size_t );
    // friend class Factory;
    friend class MachPhysObjectFactory<Id, MachPhysAdministrator>;

    //  Necessary to allow the persistence mechanism write out the factory
    friend void perWrite(PerOstream&, const MachPhysMachinePersistence&);
    friend void perRead(PerIstream&, MachPhysMachinePersistence&);

    static MachPhysAdministrator& part(MachPhys::AdministratorSubType subType, size_t bodyLevel);
    static Factory& factory();

    void createExplosionData();
    SysPathName compositeFileName(MachPhys::AdministratorSubType, size_t bodyLevel) const;

    //  This is the constructor that is used by the factory. It is the
    //  only constructor that actually builds an administrator from scratch
    MachPhysAdministrator(W4dEntity* pParent, Id id);

    MachPhys::AdministratorSubType subType_;
};

PER_DECLARE_PERSISTENT(MachPhysAdministrator);

#endif

/* End ADMINIST.HPP *************************************************/
