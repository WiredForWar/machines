/*
 * T L P O L Y . C P P
 * (c) Charybdis Limited, 1997. All Rights Reserved
 */

#include "render/internal/TTFPolygonImpl.hpp"
#include "render/internal/TLPolygon.hpp"
#include "render/internal/VertexData.hpp"

// Note: we need to copy the vertices because these might change when other
// instances of the same TTF are rendered in this frame.  The rest of the pimple
// need not be copied because instances won't affect it.
RenITLPolygon::RenITLPolygon(const RenITTFImpl* p, const RenMaterial& mat, MATHEX_SCALAR w, Ren::PrimitiveTopology topology)
    : RenIDepthSortedItem(mat)
    , pImpl_(p)
    , vertices_(_NEW_ARRAY(RenIVertex, p->nVertices()))
    , topology_(topology)
{
    PRE(p);
    depth_ = w;

    ASSERT(vertices_, "RenITLPolygon::vertices_ not initialized.");
    for (int i = 0; i != p->nVertices(); ++i)
        vertices_[i] = p->vertices()[i];
}

RenITLPolygon::~RenITLPolygon()
{
    _DELETE_ARRAY(vertices_);
}

void RenITLPolygon::render()
{
    // NB: use the material from the base class rather than the one in the TTF
    // pimple.  The former reflects any material overrides, the latter doesn't.
    pImpl_->renderGL(vertices_, material_, topology_);
}

void RenITLPolygon::print(std::ostream& o) const
{
    o << "TL polygon, depth=" << depth() << ", " << material_;
}

/* End TLPOLY.CPP ***************************************************/
