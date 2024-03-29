/*
 * D Y I N G E N T . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

/*
    MachLogDyingEntityEvent

    Holder for exploding animation so that the original MachActor may be destroyed
*/

#ifndef _MACHLOG_DYINGENT_HPP
#define _MACHLOG_DYINGENT_HPP

#include "base/base.hpp"
#include "sim/disevent.hpp"
#include "phys/phys.hpp"
#include "ftl/serialid.hpp"

// #include "machlog/actor.hpp"
#include "world4d/entity.hpp"

class MachLogConstruction;
class MachLogDyingEntityEventImpl;
template <class X> class ctl_nb_vector;

class MachLogDyingEntityEvent : public SimDiscreteEvent
{
public:
    using PolygonId = FtlSerialId;
    using Polygons = ctl_nb_vector<PolygonId>;
    enum InsideBuilding
    {
        INSIDE_BUILDING,
        NOT_INSIDE_BUILDING
    };

    MachLogDyingEntityEvent(
        const CtlConstCountedPtr<W4dEntity>& physObjectPtr,
        const Polygons*,
        const PhysRelativeTime& goDeadTime,
        InsideBuilding,
        MachLogConstruction*);

    MachLogDyingEntityEvent(
        const CtlConstCountedPtr<W4dEntity>& physObjectPtr,
        PolygonId singlePolygonOnly,
        const PhysRelativeTime& goDeadTime,
        InsideBuilding,
        MachLogConstruction*);

    ~MachLogDyingEntityEvent() override;

    void CLASS_INVARIANT;

    // inherited from SimDiscreteEvent
    // called when the time goes out of range by SimManager.
    void execute() override;

    friend std::ostream& operator<<(std::ostream& o, const MachLogDyingEntityEvent& t);
    void doOutputOperator(std::ostream&) const override;

private:
    // Operation deliberately revoked
    MachLogDyingEntityEvent(const MachLogDyingEntityEvent&);
    MachLogDyingEntityEvent& operator=(const MachLogDyingEntityEvent&);
    bool operator==(const MachLogDyingEntityEvent&);

    MachLogDyingEntityEventImpl* pImpl_;
};

PER_ENUM_PERSISTENT(MachLogDyingEntityEvent::InsideBuilding);

#endif

/* End DYINGENT.HPP *************************************************/
