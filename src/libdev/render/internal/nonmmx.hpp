/*
 * N O N M M X . H P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

/*
    RenINonMMXIlluminator

    A concrete class derived from RenIIlluminator which implements lighting
    calculations in the absence of an MMX CPU.
*/

#ifndef _RENDER_NONMMX_HPP
#define _RENDER_NONMMX_HPP

#include "base/base.hpp"
#include "render/internal/illumine.hpp"

class RenIFloatLightingBuffer;

class RenINonMMXIlluminator : public RenIIlluminator
{
public:
    RenINonMMXIlluminator(RenIDeviceImpl*);
    ~RenINonMMXIlluminator() override;

    const RenIVertex* applyMaterial(const RenMaterial&, const RenIVertexData& in, const Indices&) override;

private:
    void computeLambertian(const RenIVertexData&, const MexTransform3d& world, const MexAlignedBox3d* pVolume) override;
    void applyVertexMaterials(const RenIVertexData& in) override;
    void doStartFrame() override;
    bool overflowPossible(const RenIMatBody*) const;
    bool specularRequired() const;

    RenIFloatLightingBuffer* lightingBuffer_;
    uint32_t overflows_, applications_;
};

#endif

/* End NONMMX.HPP ***************************************************/
