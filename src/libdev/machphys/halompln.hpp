/*
 * H A L O M P L N . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    MachPhysLightningHaloMaterialPlan

    create a unique material plam for the electric lightning halo associated with the
    victim effect of an electric charger
*/

#ifndef _MACHPHYS_HALOMPLN_HPP
#define _MACHPHYS_HALOMPLN_HPP

#include "base/base.hpp"
#include "ctl/vector.hpp"
#include "ctl/countptr.hpp"
#include "render/render.hpp"
#include "machphys/halo.hpp"
#include "phys/sclplan.hpp"
#include "world4d/matplan.hpp"

class MachPhysLightningHaloMaterialPlan : public W4dMaterialPlan
// Canonical form revoked
{
public:
    // Collection of (counted pointers to) material vectors
    using MaterialVecPtrs = ctl_vector<Ren::MaterialVecPtr>;

    // public ctor
    MachPhysLightningHaloMaterialPlan(
        const PhysRelativeTime& duration,
        W4dLOD maxLOD,
        MachPhysLightningHalo::HaloType type);

    ~MachPhysLightningHaloMaterialPlan() override;

    // True if the plan is defined for level of detail lodId.
    bool isLODDefined(W4dLOD lodId) const override;

    // Override to define result as a function of timeOffset.
    const Ren::MaterialVecPtr& materialVec(const PhysRelativeTime& timeOffset, W4dLOD lodId) const override;

    void CLASS_INVARIANT;

    friend ostream& operator<<(ostream& o, const MachPhysLightningHaloMaterialPlan& t);

private:
    MachPhysLightningHaloMaterialPlan(const MachPhysLightningHaloMaterialPlan&);
    MachPhysLightningHaloMaterialPlan& operator=(const MachPhysLightningHaloMaterialPlan&);
    bool operator==(const MachPhysLightningHaloMaterialPlan&);

    RenMaterial mat1_;
    RenMaterial mat2_;
    RenMaterial mat3_;
    RenMaterial mat4_;

    PhysScalarPlanPtr alphaPlanPtr_; // Scalar plan defining alpha value

    MaterialVecPtrs materialVecPtrs_;
};

#endif

/* End HALOMPLN.HPP *************************************************/
