/*
 * G E O L O C A T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachPhysGeoLocator

    Implement the physical properties of a locator
*/

#ifndef _MACHPHYS_GEOLOCAT_HPP
#define _MACHPHYS_GEOLOCAT_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "phys/phys.hpp"
#include "machphys/machine.hpp"

class MachPhysGeoLocatorData;
class MachPhysMachinePersistence;
class MachPhysGeoLocatorImpl;
template <class ID, class PART> class MachPhysObjectFactory;

class MachPhysGeoLocator : public MachPhysMachine
{
public:
    MachPhysGeoLocator(
        W4dEntity* pParent,
        const W4dTransform3d& localTransform,
        size_t bodyLevel,
        size_t brainLevel,
        MachPhys::Race race);

    ~MachPhysGeoLocator() override;

    // Set/get locating mode
    void isLocating(bool doLocate);
    bool isLocating() const;

    // Inherited from MachPhysMachine
    void move(const MachPhysMachineMoveInfo& info) override;

    const MachPhysMachineData& machineData() const override;

    const MachPhysGeoLocatorData& data() const;

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysGeoLocator& t);

    PER_MEMBER_PERSISTENT(MachPhysGeoLocator);
    PER_FRIEND_READ_WRITE(MachPhysGeoLocator);

private:
    // Operations deliberately revoked
    MachPhysGeoLocator(const MachPhysGeoLocator&);
    MachPhysGeoLocator& operator=(const MachPhysGeoLocator&);
    bool operator==(const MachPhysGeoLocator&);

    //  This is necessary to allow the ti file to instantiate the factory class
    // friend MachPhysGeoLocator& Factory::part( const ID&, size_t );
    using Factory = MachPhysObjectFactory<size_t, MachPhysGeoLocator>;
    // friend class Factory;
    friend class MachPhysObjectFactory<size_t, MachPhysGeoLocator>;

    //  Necessary to allow the persistence mechanism write out the factory
    friend void perWrite(PerOstream&, const MachPhysMachinePersistence&);
    friend void perRead(PerIstream&, MachPhysMachinePersistence&);

    static MachPhysGeoLocator& part(size_t bodyLevel);
    static Factory& factory();

    SysPathName compositeFileName(size_t bodyLevel) const;

    //  This is the constructor that is used by the factory. It is the
    //  only constructor that actually builds an administrator from scratch

    MachPhysGeoLocator(W4dEntity* pParent, size_t bodyLevel);

    void createExplosionData();

    enum
    {
        LOCATOR_LOCATING
    };

    // Data members
    MachPhysGeoLocatorImpl* pImpl_; // implementation object
};

PER_DECLARE_PERSISTENT(MachPhysGeoLocator);

#endif

/* End GEOLOCAT.HPP *************************************************/
