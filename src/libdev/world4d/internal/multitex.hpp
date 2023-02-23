/*
 * M U L T I T E X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    W4dCycleMultiTextureData

    defines the data for a text cycling animation specified by a LOD/CDF file
*/

#ifndef _WORLD4D_MULTITEX_HPP
#define _WORLD4D_MULTITEX_HPP

#include "base/base.hpp"
#include "base/persist.hpp"
#include "phys/phys.hpp"
#include "world4d/world4d.hpp"
#include "render/render.hpp"
#include "render/texture.hpp"
#include "ctl/vector.hpp"
#include "ctl/pvector.hpp"
#include "ctl/countptr.hpp"
#include "world4d/internal/animdata.hpp"

class W4dEntity;
class W4dCycleTextureData;

class W4dCycleMultiTextureData : public W4dAnimationData
// Canonical form revoked
{
public:
    W4dCycleMultiTextureData(const size_t& nData);

    ~W4dCycleMultiTextureData() override;

    // apply the texture cucling animation to pEntity  start at statTime
    void apply(W4dEntity* pEntity, const PhysAbsoluteTime& startTime) override;

    void add(W4dCycleTextureData* pData);

    void CLASS_INVARIANT;

    PER_MEMBER_PERSISTENT_VIRTUAL(W4dCycleMultiTextureData);
    PER_FRIEND_READ_WRITE(W4dCycleMultiTextureData);

private:
    friend ostream& operator<<(ostream& o, const W4dCycleMultiTextureData& t);

    W4dCycleMultiTextureData(const W4dCycleMultiTextureData&);
    W4dCycleMultiTextureData& operator=(const W4dCycleMultiTextureData&);

    ctl_vector<ctl_vector<RenTexture>> keyTextures2d() const;

    using MaterialVecPtrs = ctl_vector<Ren::MaterialVecPtr>;
    using MaterialVecPtrsPtr = CtlCountedPtr<MaterialVecPtrs>;
    using MaterialVecPtrsPtrVec = ctl_vector<MaterialVecPtrsPtr>;

    ctl_pvector<W4dCycleTextureData> cycleTextureDataVec_;
};

PER_DECLARE_PERSISTENT(W4dCycleMultiTextureData);

#endif

/* End MULTITEX.HPP **************************************************/
