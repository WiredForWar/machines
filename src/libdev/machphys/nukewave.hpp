/*
 * N U K E W A V E. H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysNukeWave

    Models part of a vortex weapon bomb explosion animation
*/

#ifndef _NUKEWAVE_HPP
#define _NUKEWAVE_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "phys/phys.hpp"
#include "render/render.hpp"
#include "world4d/entity.hpp"
#include "machphys/flames.hpp"

class MachPhysNukeWave : public W4dEntity
// Canonical form revoked
{
public:
    // ctor
    MachPhysNukeWave(W4dEntity* pParent, const MexTransform3d& localTransform);

    // Return an exemplar  - ensures the bomb meshes and textures are loaded
    static const MachPhysNukeWave& exemplar();

    // start the shock wave as planned
    void startNukeWave(
        const PhysAbsoluteTime& startTime,
        const PhysRelativeTime& duration,
        const MATHEX_SCALAR& fromRadius,
        const MATHEX_SCALAR& toRadius,
        const MATHEX_SCALAR& zScale);
    // dtor
    ~MachPhysNukeWave() override;

    // Inherited from W4dEntity. Returns false.
    bool intersectsLine(const MexLine3d& line, MATHEX_SCALAR* pDistance, Accuracy accuracy) const override;

    // shockwave reaches distance at *pTime. return true if distance <= shockwave radius
    static bool reachDistance(
        const PhysRelativeTime& duration,
        const MATHEX_SCALAR& fromRadius,
        const MATHEX_SCALAR& toRadius,
        const MATHEX_SCALAR& distance,
        PhysRelativeTime* pTime);

    static const double& waveDefaultSize();

    using Materials = MachPhysFlame::Materials;
    static void addMaterial(MachPhysNukeWave::Materials*, const RenTexture&);

    void CLASS_INVARIANT;

    friend std::ostream& operator<<(std::ostream& o, const MachPhysNukeWave& t);

    PER_MEMBER_PERSISTENT(MachPhysNukeWave);

private:
    // Deliberately revoked
    MachPhysNukeWave(const MachPhysNukeWave&);
    MachPhysNukeWave& operator=(const MachPhysNukeWave&);
    bool operator==(const MachPhysNukeWave&);

    friend class MachPhysOtherPersistence;
    friend class MachPhysPreload;
    // One-time constructor used to create the exemplar
    MachPhysNukeWave();

    void setMaterialFogMultipliers();

    // typedef  MachPhysFlame::Materials Materials;
    using MaterialsPtr = MachPhysFlame::MaterialsPtr;

    static const MaterialsPtr& materialsPtr();
};

PER_READ_WRITE(MachPhysNukeWave);
PER_DECLARE_PERSISTENT(MachPhysNukeWave);

#endif

/* End NUKEWAVE.HPP *************************************************/
