/*
 * A L P H A G R P . H P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#ifndef _RENDER_DELAYALPHAGRP_HPP
#define _RENDER_DELAYALPHAGRP_HPP

#include "base/base.hpp"
#include "render/material.hpp"
#include "render/internal/internal.hpp"
#include "render/internal/polysord.hpp"
#include <glm/gtc/matrix_transform.hpp>

class RenIMaterialGroup;
class RenIVertexData;

// A RenIMaterialGroup which cannot be drawn in order.  These are pushed onto a
// list, then sorted and drawn after all other rendering is complete.
class RenIDelayedAlphaGroup : public RenIDepthSortedItem
{
public:
    RenIDelayedAlphaGroup(const RenIMaterialGroup*, RenI::LitVtxAPtr, const RenMaterial&, const glm::mat4&);

    ~RenIDelayedAlphaGroup() override;
    void render() override;

protected:
    const RenIMaterialGroup* group_;
    RenI::LitVtxAPtr vertices_;
    const glm::mat4 xform_;

    // Operations deliberately revoked.
    RenIDelayedAlphaGroup(const RenIDelayedAlphaGroup&);
    RenIDelayedAlphaGroup& operator=(const RenIDelayedAlphaGroup&);
    bool operator==(const RenIDelayedAlphaGroup&);

    void print(std::ostream&) const override;
};

#endif

/* End DELAYGRP.HPP *************************************************/
