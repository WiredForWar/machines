/*
 * A L P H A G R P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_DELAYALPHAGRP_HPP
#define _RENDER_DELAYALPHAGRP_HPP

#include "base/base.hpp"
#include "render/Material.hpp"
#include "render/internal/internal.hpp"
#include "render/internal/PolySortedDraw.hpp"
#include "render/internal/GpuMeshLightingSnapshot.hpp"
#include <glm/gtc/matrix_transform.hpp>

class RenIMaterialGroup;

// A RenIMaterialGroup which cannot be drawn in order.  These are pushed onto a
// list, then sorted and drawn after all other rendering is complete.
class RenIDelayedAlphaGroup : public RenIDepthSortedItem
{
public:
    RenIDelayedAlphaGroup(const RenIMaterialGroup*, RenI::LitVtxAPtr, const RenMaterial&, const glm::mat4&, RenI::GpuMeshLightingSnapshot gpuSnapshot = {});

    ~RenIDelayedAlphaGroup() override;
    void render() override;

protected:
    const RenIMaterialGroup* group_;
    RenI::LitVtxAPtr vertices_;
    const glm::mat4 xform_;
    RenI::GpuMeshLightingSnapshot gpuSnapshot_;

    // Operations deliberately revoked.
    RenIDelayedAlphaGroup(const RenIDelayedAlphaGroup&);
    RenIDelayedAlphaGroup& operator=(const RenIDelayedAlphaGroup&);
    bool operator==(const RenIDelayedAlphaGroup&) const;

    void print(std::ostream&) const override;
};

#endif

/* End DELAYGRP.HPP *************************************************/
